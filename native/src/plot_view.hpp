// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "canvas.hpp"
#include "extensions.hpp"

namespace physik {
struct PlotBounds {double xmin=0,xmax=1,ymin=0,ymax=1;};
bool plotBounds(const Plot& plot,PlotBounds& bounds,std::string& error);
bool validPlotBounds(const PlotBounds& bounds);
void drawPlot(Canvas& canvas,const Plot& plot,const PlotBounds& bounds,size_t cursor);
size_t plotCursor(const Plot& plot,const PlotBounds& bounds,int pixelX);
}
