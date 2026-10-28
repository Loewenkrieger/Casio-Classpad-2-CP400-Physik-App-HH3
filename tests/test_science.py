"""Scientific and UI regressions ported from the HH2 test suite; public data only."""
from contextlib import contextmanager
import math
import unittest
from unittest.mock import patch

from support import FileSystem, event, gint, load_app, no_drawing

APP = None
GROUPS = None


@contextmanager
def overrides(**names):
    previous = {key: APP[key] for key in names}
    APP.update(names)
    try:
        yield
    finally:
        APP.update(previous)


class ScienceTests(unittest.TestCase):
    def setUp(self):
        global APP, GROUPS
        APP = load_app()
        FileSystem().attach(APP)
        GROUPS = [("Messreihen", [row for row in APP["CALCS"] if row[-1].startswith("FIT_")])]

    def assertClose(self, actual, expected):
        self.assertTrue(math.isclose(actual, expected, rel_tol=2e-9, abs_tol=0), repr((actual, expected)))

    def q(self, ident, values):
        return APP["calc_value"]("Q_" + ident, values)

    def test_reference_values(self):
        e = APP['E_CHARGE']
        checks = [('EL', [5e-07], 2.479683968664005 * e), ('LUC', [500], 5.484743655667948e-11), ('SVE', [1e-06], 57.88381798194462), ('KL', [5.5e-07, 2.4e-19], 0.7562959754603306 * e), ('BOX_E', [1e-09, 1], 0.37603016210487256 * e), ('H_PH', [1, 3, 2], 6.563869328816485e-07), ('XR', [30000], 4.132806614440009e-11), ('EXL', [4.9], 2.5302897639428624e-07), ('COMP', [90], 2.426310235380317e-12)]
        for ident, values, expected in checks:
            with self.subTest(ident=ident):
                self.assertClose(self.q(ident, values), expected)

    def test_all_quantum_equations_independently(self):
        h, e, c, m = (APP['H_PLANCK'], APP['E_CHARGE'], APP['LIGHT_C'], APP['M_E'])
        a, n, k = (1e-09, 2, 2 * e)
        f, lam = (600000000000000.0, 5e-07)
        box = h ** 2 / (8 * m * a * a)
        cases = {'EF': ([f], h * f), 'EL': ([lam], h * c / lam), 'FE': ([k], k / h), 'LE': ([k], h * c / k), 'PL': ([lam], h / lam), 'PE': ([k], k / c), 'EP': ([h / lam], h * c / lam), 'LP': ([h / lam], lam), 'NE': ([2, k], 2 / k), 'EN': ([2, k], 2 * k), 'NPT': ([2, 3, k], 6 / k), 'PN': ([2, k, 3], 2 * k / 3), 'TN': ([2, k, 3], 2 * k / 3), 'KF': ([f, e], h * f - e), 'KL': ([lam, e], h * c / lam - e), 'UK': ([k], 2), 'KU': ([2], k), 'WF': ([f, 1], h * f - e), 'WL': ([lam, e], h * c / lam - e), 'FG': ([k], k / h), 'LG': ([k], h * c / k), 'WFG': ([f], h * f), 'WLG': ([lam], h * c / lam), 'VK': ([k], math.sqrt(2 * k / m)), 'FKW': ([e, e], 2 * e / h), 'LKW': ([e, e], h * c / (2 * e)), 'DB': ([h / lam], lam), 'PD': ([lam], h / lam), 'LMV': ([m, 1000], h / (m * 1000)), 'VD': ([m, h / (m * 1000)], 1000), 'MD': ([1000, h / (m * 1000)], m), 'LK': ([m, k], h / math.sqrt(2 * m * k)), 'KD': ([m, lam], h * h / (2 * m * lam * lam)), 'EU': ([500], e * 500), 'LUC': ([500], h / math.sqrt(2 * m * e * 500)), 'LUR': ([100000], h * c / math.sqrt(e * 100000 * (e * 100000 + 2 * m * c * c))), 'UD': ([lam], h * h / (2 * m * e * lam * lam)), 'BL': ([2e-10, 30, 1], 2e-10), 'BD': ([2e-10, 30, 1], 2e-10), 'BT': ([2e-10, 2e-10, 1], 30), 'RL': ([2e-10, 0.02, 0.1, 1], 4e-10 * math.sin(math.atan(0.2) / 2)), 'RR': ([4e-11, 2e-10, 0.1, 1], 0.1 * math.tan(2 * math.asin(0.1))), 'DS': ([lam, 1, 0.0005], 0.001), 'DL': ([0.001, 0.0005, 1], lam), 'DD': ([lam, 1, 0.001], 0.0005), 'DDIST': ([0.001, 0.0005, lam], 1), 'SP': ([1e-06], h / (4 * math.pi * 1e-06)), 'SX': ([h / (4 * math.pi * 1e-06)], 1e-06), 'SV': ([m, 1e-06], h / (4 * math.pi * m * 1e-06)), 'SVE': ([1e-06], h / (4 * math.pi * m * 1e-06)), 'SXV': ([m, 100], h / (4 * math.pi * m * 100)), 'SF': ([5e-15], 1 / (4 * math.pi * 5e-15)), 'ST': ([100000000000000.0], 1 / (4 * math.pi * 100000000000000.0)), 'GF': ([5e-15], 2 * math.log(2) / (math.pi * 5e-15)), 'GT': ([100000000000000.0], 2 * math.log(2) / (math.pi * 100000000000000.0)), 'SCHOOL': ([5e-15], 200000000000000.0), 'SCHOOLT': ([200000000000000.0], 5e-15), 'GAMMA': ([1e-09], h / (2 * math.pi * 1e-09)), 'TAU': ([e], h / (2 * math.pi * e)), 'BOX_L': ([a, n], a), 'BOX_P': ([a, n], h / a), 'BOX_E': ([a, n], 4 * box), 'BOX_EM': ([a, n, 2 * m], 2 * box), 'BOX_A': ([4 * box, n], a), 'BOX_D': ([a, 2, 1], 3 * box), 'BOX_PH': ([a, 2, 1], h * c / (3 * box)), 'BOHR': ([2, 2], -13.6 * e), 'ION': ([2, 2], 13.6 * e), 'RAD': ([2, 2], 2 * APP['BOHR_A']), 'H_D': ([1, 3, 2], 13.6 * e * (1 / 4 - 1 / 9)), 'H_PH': ([1, 3, 2], h * c / (13.6 * e * (1 / 4 - 1 / 9))), 'LEVEL_D': ([-3 * e, -e], 2 * e), 'LEVEL_L': ([-3 * e, -e], h * c / (2 * e)), 'LEVEL_F': ([-3 * e, -e], 2 * e / h), 'EX': ([4.9], 4.9 * e), 'EXL': ([4.9], h * c / (4.9 * e)), 'EXU': ([4.9 * e], 4.9), 'XR': ([30000], h * c / (30000 * e)), 'XU': ([4e-11], h * c / (4e-11 * e)), 'COMP': ([90], h / (m * c)), 'COMPL': ([4e-11, 90], 4e-11 + h / (m * c))}
        ids = {row[-1][2:] for row in tuple(APP['CALCS'])[82:] if row[-1].startswith('Q_')}
        self.assertEqual(ids, set(cases))
        for ident, (values, expected) in cases.items():
            with self.subTest(ident=ident):
                self.assertClose(self.q(ident, values), expected)

    def test_invalid_physics(self):
        invalid = [('EL', [0]), ('EL', [-1]), ('KF', [100000000000000.0, 5e-19]), ('WF', [100000000000000.0, 10]), ('BOX_E', [1e-09, 0]), ('BOX_E', [1e-09, 1.5]), ('H_PH', [1, 2, 2]), ('BOHR', [0, 1]), ('BT', [5e-10, 1e-10, 1]), ('BL', [1e-10, 120, 1]), ('COMP', [181]), ('RR', [3e-10, 2e-10, 0.1, 1]), ('LUC', [100000]), ('VD', [1e-31, 1e-15]), ('LEVEL_L', [-1, -1]), ('SP', [float('inf')]), ('LE', [float('nan')])]
        for ident, values in invalid:
            with self.subTest(ident=ident), self.assertRaises((ValueError, OverflowError)):
                self.q(ident, values)

    def test_units_and_small_numbers(self):
        for unit, options in APP['UNITS'].items():
            for i, (label, scale) in enumerate(options):
                with self.subTest(unit=label):
                    self.assertClose(APP['scaled_input']('2', options, i), 2 * scale)
        for x in (6.626e-34, -6.626e-34, 1e-12, 1e+30):
            self.assertNotEqual(APP['format_number'](x), '0')
        for text in ('nan', 'inf', '1E999', '1E-999'):
            with self.assertRaises(ValueError):
                APP['scaled_input'](text, (('m', 1),), 0)
        self.assertEqual(APP['append_number_char']('1E2', '.'), '1E2')

    def test_planck_fits(self):
        e = APP['E_CHARGE']
        points = [(x * 1e-09, y * e) for x, y in zip((486, 434, 410, 397), (0.61, 0.92, 1.09, 1.19))]
        h, w, f, rms = APP['fit_planck'](points, True)
        self.assertClose(h, 6.721770195411257e-34)
        self.assertClose(w / e, 1.9779534431652634)
        self.assertClose(f, w / h)
        for count in (2, 3, 10):
            points = [(500000000000000.0 + i * 10000000000000.0, APP['H_PLANCK'] * (500000000000000.0 + i * 10000000000000.0) / e - 2) for i in range(count)]
            h, w, _, r = APP['fit_planck'](points)
            self.assertClose(h, APP['H_PLANCK'])
            self.assertClose(w, 2 * e)
            self.assertLess(r, 1e-13)
        for points in ([(1, 1)], [(1, 1), (1, 2)], [(1, 2), (2, 1)], [(1, 2), (2, 3)]):
            with self.assertRaises(ValueError):
                APP['fit_planck'](points)

    def test_fit_user_flow(self):
        e = APP['E_CHARGE']
        h = APP['H_PLANCK']
        for calc in GROUPS[-1][1]:
            wavelength = calc[-1].startswith('FIT_L')
            count = 3 if calc[-1].endswith('N') else 2
            inputs = [count] if count == 3 else []
            for i in range(count):
                f = 600000000000000.0 + i * 100000000000000.0
                inputs.extend((APP['LIGHT_C'] / f if wavelength else f, h * f - 2 * e if wavelength else h * f / e - 2))
            inputs = iter(inputs)
            output = []
            with overrides(input_number=lambda *args: next(inputs), show_text=lambda *args: output.append(args)):
                APP['run_calc'](calc)
            self.assertIn('h_fit', output[0][1])
            self.assertIn('W_A', output[0][1])
            self.assertFalse(output[0][1].startswith('Fehler:'))
        output = []
        with overrides(input_number=lambda *args: 3.5, show_text=lambda *args: output.append(args)):
            APP['run_calc'](GROUPS[-1][1][1])
        self.assertIn('3 bis 10', output[0][1])

    def test_article_returns_to_scroll_position(self):
        chapter = next(iter(APP['ARTICLE_MEDIA']))
        frames = []
        opened = []
        queue = iter(([event(gint.KEY_DOWN)], [event(gint.KEY_EXE)], [event(gint.KEY_EXIT)]))

        def text(x, y, *args):
            if args[-2] not in ('Abbildungen (EXE)',):
                frames.append((y, args[-2]))
        with overrides(**dict(no_drawing(), poll_events=lambda: next(queue), keypressed=lambda k: False, dtext_opt=text, show_media=lambda ids: opened.append(ids))):
            APP['show_article']('Test', chapter)
        self.assertEqual(opened, [APP['ARTICLE_MEDIA'][chapter]])
        texts = [(y, t) for y, t in frames if t == 'Kernaussage:']
        self.assertEqual([y for y, t in texts], [50, 32, 32])

    def test_viewer_redraw_navigation_and_cancel(self):
        drawn = []
        queue = iter(([], [event(gint.KEY_RIGHT)], [event(x=5, y=500)], [event(x=260, y=500)]))
        with overrides(poll_events=lambda: next(queue), render_media_page=lambda i, p, n: drawn.append(i)):
            APP['show_media'](('probe_vector', 'probe_p16'))
        self.assertEqual(drawn, ['probe_vector', 'probe_p16', 'probe_vector'])
        errors = []

        def fail(*args):
            raise ValueError('beschaedigt')
        with overrides(render_media_page=fail, show_text=lambda *args: errors.append(args)):
            APP['show_media'](('probe_vector',))
        self.assertIn('beschaedigt', errors[0][1])

    def test_input_unit_touch_and_cancel(self):

        def touch(row, col):
            return event(x=8 + col * 78 + 20, y=250 + row * 58 + 20)
        queue = iter(([touch(2, 1)], [touch(3, 3)], [touch(3, 2)]))
        with overrides(**dict(no_drawing(), poll_events=lambda: next(queue))):
            self.assertClose(APP['input_number']('lambda', 'm', 'Test'), 2e-09)
        queue = iter(([event(gint.KEY_EXIT)],))
        with overrides(**dict(no_drawing(), poll_events=lambda: next(queue))):
            self.assertIsNone(APP['input_number']('x', 'm', 'Test'))
