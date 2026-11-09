"""Translate the two fixed Python formula dispatchers to build-time C++ only."""
import ast
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]


class Translator:
    def expression(self, node):
        e = self.expression
        if isinstance(node, ast.Constant):
            return json.dumps(node.value) if isinstance(node.value, str) else repr(float(node.value))
        if isinstance(node, ast.Name):
            return 'ELECTRON_MASS' if node.id == 'M_E' else node.id
        if isinstance(node, ast.Subscript):
            if isinstance(node.slice, ast.Slice):
                assert isinstance(node.slice.lower, ast.Constant) and node.slice.lower.value == 2
                return e(node.value) + '.substr(2)'
            if isinstance(node.slice, ast.UnaryOp):
                assert isinstance(node.slice.op, ast.USub) and node.slice.operand.value == 1
                return e(node.value) + '.back()'
            return e(node.value) + '[' + str(node.slice.value) + ']'
        if isinstance(node, ast.BinOp):
            if isinstance(node.op, ast.Pow):
                return 'std::pow(' + e(node.left) + ',' + e(node.right) + ')'
            op = {ast.Add: '+', ast.Sub: '-', ast.Mult: '*', ast.Div: '/'}[type(node.op)]
            return '(' + e(node.left) + op + e(node.right) + ')'
        if isinstance(node, ast.UnaryOp):
            return ({ast.Not: '!', ast.USub: '-', ast.UAdd: '+'}[type(node.op)] + '(' + e(node.operand) + ')')
        if isinstance(node, ast.IfExp):
            return '(' + e(node.test) + '?' + e(node.body) + ':' + e(node.orelse) + ')'
        if isinstance(node, ast.Compare):
            left, parts = node.left, []
            for op, right in zip(node.ops, node.comparators):
                if isinstance(op, (ast.In, ast.NotIn)):
                    test = '(' + '||'.join(e(left) + '==' + e(item) for item in right.elts) + ')'
                    parts.append('!' + test if isinstance(op, ast.NotIn) else test)
                else:
                    parts.append('(' + e(left) + {ast.Eq: '==', ast.NotEq: '!=', ast.Lt: '<', ast.LtE: '<=', ast.Gt: '>', ast.GtE: '>='}[type(op)] + e(right) + ')')
                left = right
            return '(' + '&&'.join(parts) + ')'
        if isinstance(node, ast.Call):
            if isinstance(node.func, ast.Attribute):
                if node.func.attr == 'startswith':
                    return '(' + e(node.func.value) + '.rfind(' + e(node.args[0]) + ',0)==0)'
                assert isinstance(node.func.value, ast.Name) and node.func.value.id == 'math'
                name = 'need_sqrt' if node.func.attr == 'sqrt' else 'std::' + node.func.attr
            else:
                name = {'abs': 'std::abs', 'quantum_value': 'quantum'}.get(node.func.id, node.func.id)
            return name + '(' + ','.join(e(arg) for arg in node.args) + ')'
        raise ValueError('Unsupported formula expression: ' + ast.dump(node))

    def statements(self, nodes, known, indent=2):
        lines, pad = [], ' ' * indent
        for node in nodes:
            if isinstance(node, ast.Assign):
                name = node.targets[0].id
                prefix = '' if name in known else ('const auto& ' if name == 'v' else 'auto ')
                lines.append(pad + prefix + name + '=' + self.expression(node.value) + ';')
                known.add(name)
            elif isinstance(node, ast.AugAssign):
                op = {ast.Div: '/=', ast.Mult: '*=', ast.Add: '+=', ast.Sub: '-='}[type(node.op)]
                lines.append(pad + self.expression(node.target) + op + self.expression(node.value) + ';')
            elif isinstance(node, ast.If):
                lines.append(pad + 'if(' + self.expression(node.test) + '){')
                lines += self.statements(node.body, set(known), indent + 2)
                if node.orelse:
                    lines.append(pad + '}else{')
                    lines += self.statements(node.orelse, set(known), indent + 2)
                lines.append(pad + '}')
            elif isinstance(node, ast.For):
                lines.append(pad + 'for(double ' + node.target.id + ':' + self.expression(node.iter) + '){')
                lines += self.statements(node.body, set(known) | {node.target.id}, indent + 2)
                lines.append(pad + '}')
            elif isinstance(node, ast.Return):
                lines.append(pad + 'return ' + self.expression(node.value) + ';')
            elif isinstance(node, ast.Raise):
                lines.append(pad + 'return fail(' + self.expression(node.exc.args[0]) + ');')
            elif isinstance(node, ast.Expr):
                lines.append(pad + self.expression(node.value) + ';')
            else:
                raise ValueError('Unsupported formula statement: ' + ast.dump(node))
        return lines


def main():
    tree = ast.parse((ROOT.parent / 'physchem_final.py').read_text(encoding='ascii'))
    output = ['// Generated fixed C++ equations from the unchanged Python reference.']
    for source, target, argument in [('calc_value', 'classic', 'calc_id'), ('quantum_value', 'quantum', 'ident')]:
        function = next(node for node in tree.body if isinstance(node, ast.FunctionDef) and node.name == source)
        output.append('double ' + target + '(const std::string& ' + argument + ',const std::vector<double>& values){')
        output.extend(Translator().statements(function.body, {argument, 'values'}))
        output.append('}')
    sys.path.insert(0, str(ROOT.parent / 'tests'))
    from support import load_app
    output.append('int expectedCount(const std::string& id){')
    for row in load_app()['CALCS']:
        if not row[-1].startswith('FIT_'):
            output.append('  if(id==' + json.dumps(row[-1]) + ')return ' + str(len(row[4])) + ';')
    output.extend(['  return -1;', '}'])
    path = ROOT / 'src/equations.inc'
    text = '\n'.join(output) + '\n'
    if not path.exists() or path.read_text(encoding='ascii') != text:
        path.write_text(text, encoding='ascii', newline='\n')
    print('Fixed native equations generated; no runtime Python required')


if __name__ == '__main__':
    main()
