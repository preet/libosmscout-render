#ifndef OSMSCOUTRENDER_VEC3_HPP
#define OSMSCOUTRENDER_VEC3_HPP

#include <math.h>

namespace osmscout
{
    // cartesian coordinate system vector class
    
    class Vec3
    {
    public:
        Vec3() :
            x(0),y(0),z(0) {}

        Vec3(double myX, double myY, double myZ) :
            x(myX),y(myY),z(myZ) {}

        inline double Dot(Vec3 const & otherVec) const
        {
            return (x*otherVec.x)+
                   (y*otherVec.y)+
                   (z*otherVec.z);
        }

        inline Vec3 Cross(Vec3 const & otherVec) const
        {
            return Vec3((y*otherVec.z - z*otherVec.y),
                        (z*otherVec.x - x*otherVec.z),
                        (x*otherVec.y - y*otherVec.x));
        }

        inline double DistanceTo(Vec3 const &otherVec) const
        {
            return sqrt((x-otherVec.x)*(x-otherVec.x) +
                        (y-otherVec.y)*(y-otherVec.y) +
                        (z-otherVec.z)*(z-otherVec.z));
        }

        inline double Distance2To(Vec3 const &otherVec) const
        {
            return ((x-otherVec.x)*(x-otherVec.x) +
                    (y-otherVec.y)*(y-otherVec.y) +
                    (z-otherVec.z)*(z-otherVec.z));
        }

        inline Vec3 Normalized() const
        {      
            double vecMagnitude = sqrt(x*x + y*y + z*z);

            return Vec3(x/vecMagnitude,
                        y/vecMagnitude,
                        z/vecMagnitude);
        }

        inline Vec3 ScaledBy(double scaleFactor) const
        {
            return Vec3(x*scaleFactor,
                        y*scaleFactor,
                        z*scaleFactor);
        }

        inline Vec3 operator+ (const Vec3 &otherVec) const
        {
            return Vec3(x+otherVec.x,
                        y+otherVec.y,
                        z+otherVec.z);
        }

        inline Vec3 operator- (const Vec3 &otherVec) const
        {
            return Vec3(x-otherVec.x,
                        y-otherVec.y,
                        z-otherVec.z);
        }

        double x;
        double y;
        double z;
    };
}

#endif
