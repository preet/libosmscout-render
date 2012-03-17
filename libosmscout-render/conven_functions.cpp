    // calcMinPointLineDistance
    double calcMinPointLineDistance(Vec2 const &distalPoint,
                                    Vec2 const &endPointA,
                                    Vec2 const &endPointB);

    // calcMinPointRectDistance
    double calcMinPointRectDistance(Vec2 const &distalPoint,
                                    Vec2 const &rectBL,
                                    Vec2 const &rectTR);

    // calcMaxPointRectDistance
    double calcMaxPointRectDistance(Vec2 const &distalPoint,
                                    Vec2 const &rectBL,
                                    Vec2 const &rectTR);

    // calcRectIntersection
    bool calcRectIntersection(Vec2 const &rect1BL,Vec2 const &rect1TR,
                              Vec2 const &rect2BL,Vec2 const &rect2TR,
                              Vec2 &rectXBL,Vec2 &rectXTR);
                              
                              
double MapRenderer::calcMinPointLineDistance(const Vec2 &distalPoint,
                                             const Vec2 &endPointA,
                                             const Vec2 &endPointB)
{
    // ref: http://paulbourke.net/geometry/pointline/

    double uDenr = (endPointA.x-endPointB.x)*(endPointA.x-endPointB.x) +
            (endPointA.y-endPointB.y)*(endPointA.y-endPointB.y);

    if(uDenr == 0)
    {   // indicates that pointA and pointB are coincident
        return sqrt((distalPoint.x-endPointA.x)*(distalPoint.x-endPointA.x) +
                    (distalPoint.y-endPointA.y)*(distalPoint.y-endPointA.y));
    }

    double uNumr = (distalPoint.x-endPointA.x)*(endPointB.x-endPointA.x) +
            (distalPoint.y-endPointA.y)*(endPointB.y-endPointA.y);

    double u = uNumr/uDenr;

    if(u > 0 && u <= 1)
    {
        // indicates the projected point lies between
        // the given line segment endpoints
        double pX = endPointA.x + u*(endPointB.x-endPointA.x);
        double pY = endPointA.y + u*(endPointB.y-endPointA.y);
        return sqrt((distalPoint.x-pX)*(distalPoint.x-pX) +
                    (distalPoint.y-pY)*(distalPoint.y-pY));
    }
    else
    {
        // indicates the projected point lies outside
        // the given line segment endpoints
        double distA = sqrt((distalPoint.x-endPointA.x)*(distalPoint.x-endPointA.x) +
                            (distalPoint.y-endPointA.y)*(distalPoint.y-endPointA.y));

        double distB = sqrt((distalPoint.x-endPointB.x)*(distalPoint.x-endPointB.x) +
                            (distalPoint.y-endPointB.y)*(distalPoint.y-endPointB.y));

        return (distA < distB) ? distA : distB;
    }
}

double MapRenderer::calcMinPointRectDistance(const Vec2 &distalPoint,
                                             const Vec2 &rectBL,
                                             const Vec2 &rectTR)
{
    // first check if the point is inside the rectangle,
    // and return zero if it is
    if(distalPoint.x >= rectBL.x && distalPoint.x <= rectTR.x)
    {
        if(distalPoint.y >= rectBL.y && distalPoint.y <= rectTR.y)
        {
            return 0;
        }
    }

    // find the minimum of the perpendicular distances between the
    // given point and each of the rectangle line segments
    double distLeft,distRight,distTop,distBottom;

    distLeft = calcMinPointLineDistance(distalPoint,
                                        Vec2(rectBL.x,rectBL.y),
                                        Vec2(rectBL.x,rectTR.y));

    distRight = calcMinPointLineDistance(distalPoint,
                                         Vec2(rectTR.x,rectBL.y),
                                         Vec2(rectTR.x,rectTR.y));

    distTop = calcMinPointLineDistance(distalPoint,
                                       Vec2(rectBL.x,rectTR.y),
                                       Vec2(rectTR.x,rectTR.y));

    distBottom = calcMinPointLineDistance(distalPoint,
                                          Vec2(rectBL.x,rectBL.y),
                                          Vec2(rectTR.x,rectBL.y));

    double minDist = distLeft;

    if(distRight < minDist)
    {   minDist = distRight;   }

    if(distTop < minDist)
    {   minDist = distTop;   }

    if(distBottom < minDist)
    {   minDist = distBottom;   }

    return minDist;
}

double MapRenderer::calcMaxPointRectDistance(const Vec2 &distalPoint,
                                             const Vec2 &rectBL,
                                             const Vec2 &rectTR)
{
    // return the max distance between the point and all
    // four corners of the rectangle

    double distBL, distBR, distTL, distTR;
    distBL = sqrt((distalPoint.x-rectBL.x)*(distalPoint.x-rectBL.x)+
                  (distalPoint.y-rectBL.y)*(distalPoint.y-rectBL.y));

    distBR = sqrt((distalPoint.x-rectTR.x)*(distalPoint.x-rectTR.x)+
                  (distalPoint.y-rectBL.y)*(distalPoint.y-rectBL.y));

    distTL = sqrt((distalPoint.x-rectBL.x)*(distalPoint.x-rectBL.x)+
                  (distalPoint.y-rectTR.y)*(distalPoint.y-rectTR.y));

    distTR = sqrt((distalPoint.x-rectTR.x)*(distalPoint.x-rectTR.x)+
                  (distalPoint.y-rectTR.y)*(distalPoint.y-rectTR.y));

    double maxDist = distBL;

    if(distBR > maxDist)
    {   maxDist = distBR;   }

    if(distTL > maxDist)
    {   maxDist = distTL;   }

    if(distTR > maxDist)
    {   maxDist = distTR;   }

    return maxDist;
}

bool MapRenderer::calcRectIntersection(const Vec2 &rect1BL, const Vec2 &rect1TR,
                                       const Vec2 &rect2BL, const Vec2 &rect2TR,
                                       Vec2 &rectXBL, Vec2 &rectXTR)
{
    // check to see if rect1 and rect2 intersect
    if((rect1TR.x < rect2BL.x) ||
       (rect1BL.x > rect2TR.x) ||
       (rect1TR.y < rect2BL.y) ||
       (rect1BL.y > rect2TR.y))
    {   return false;   }

    // find the intersecting rectangle
    rectXBL.x = std::max(rect1BL.x,rect2BL.x);
    rectXBL.y = std::max(rect1BL.y,rect2BL.y);
    rectXTR.x = std::min(rect1TR.x,rect2TR.x);
    rectXTR.y = std::min(rect1TR.y,rect2TR.y);

    return true;
}
