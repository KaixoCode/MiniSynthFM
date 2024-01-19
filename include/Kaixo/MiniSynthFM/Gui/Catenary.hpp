#pragma once

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class Catenary {
    public:

        // ------------------------------------------------

        Catenary(float x0, float y0, float x1, float y1, float addLength, int numIterations = 1);

        // ------------------------------------------------

        float calcY(float x) const;

        // ------------------------------------------------

    private:
        float a; // curvature = radius of the circle fitting inside the curve at the vertex
        float inva; // inverse of a to avoid a division per point
        float b; // x offset. xpos of the vertex
        float c; // y offset. note that cosh(0) = 1

        // ------------------------------------------------

    };
    
    // ------------------------------------------------

}
    
// ------------------------------------------------
