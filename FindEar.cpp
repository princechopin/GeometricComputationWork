/*******************************************************************************************
 * 
 *  
 *  Code Challenge: C++ Geometry done for Quilter.AI.
 *  I tried to use the primitive boost::cover_by to find if a segment polygon is 
 *  convered by Polygon, but boost::geometry::cover_by does not support segement,
 *  and it does not work correctly on polygon cover polygons, so, I wrote a more 
 *  exhausted code leveraging covered_by(point, polygon) using segment decomposition.
 *  It works well for the give testcases, which are actually quite comlicated.
 *  Code prepared by Joe Wang. July 12, 2024
 */
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <boost/geometry.hpp>
#include <boost/geometry/algorithms/covered_by.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/geometries.hpp>

#define THRESHOLD 0.000001

using namespace std;
namespace bg = boost::geometry;

struct dpolygon_t; // declaration

struct dpoint_t {
  dpoint_t(const double x, const double y) :_x(x), _y(y) {}
  dpoint_t(const dpoint_t &pt) :_x(pt._x), _y(pt._y) {}
  ~dpoint_t() {}

  dpoint_t operator = (const dpoint_t &p) { _x = p._x; _y = p._y; return *this; }
  double distX(const dpoint_t &p) const { return p._x - this->_x; }
  double distY(const dpoint_t &p) const { return p._y - this->_y; }
  bool operator == (const dpoint_t &p) { return(_x == p._x && _y == p._y); }
  double _x;
  double _y;
};

struct dpRing_t {
  dpRing_t(const double x, const double y) : _pt(x, y), _next(nullptr), _prev(nullptr) {}
  ~dpRing_t() {}
  dpoint_t _pt;
  dpRing_t *_next;
  dpRing_t *_prev;
};

struct dpolygon_t {
  dpolygon_t(): _numPoints(0), _ringStart(nullptr) {}
  ~dpolygon_t() {
    if (_ringStart == nullptr) return;
    dpRing_t *ringStart = _ringStart;
    dpRing_t *ringScan = ringStart, *ringNext;
    do {
      ringNext = ringScan->_next;
      delete ringScan;
      ringScan = ringNext;
    } while (ringStart != ringStart);
    _numPoints = 0;
    _ringStart = nullptr;
    return;
  }
  
  void checkLink();
  double getArea();
  void removeVetex(dpRing_t *vertex) {
     if (vertex == nullptr) return;
     _numPoints--;
     dpRing_t *prev = vertex->_prev;
     prev->_next = vertex->_next;
     vertex->_next->_prev = prev;
     if (_ringStart == vertex) {
        _ringStart = (prev == _ringStart)?nullptr : prev;
     }
     delete vertex;
  }
  bool lineIsInside(const dpoint_t &p1, const dpoint_t &p2);
  bool lineIntersect(dpoint_t &p1, dpoint_t &p2, dpoint_t &p3, dpoint_t &p4);

  int _numPoints;
  dpRing_t *_ringStart;
};

struct dtriangle_t {
  dtriangle_t(const dpoint_t &p1, const dpoint_t &p2, const dpoint_t &p3) : _p1(p1), _p2(p2), _p3(p3) {}
  ~dtriangle_t() {}
  dpoint_t _p1, _p2, _p3;
  double getArea();
};

class FindEar {
  public:
   FindEar() : _totalArea(0.0) {}
   ~FindEar() {}

  void getPolygon(const char *fileName);
  int run();

  private:

    dpolygon_t _polygonIn;  
    vector<dtriangle_t> _triangles;
    double _totalArea;
};

int main(int argc, char **argv) {
  if (argc <= 1 || argc > 2) {
    cout << "Usage:FindEar <polygonFile>\n";
    return 1;
  } 

  FindEar findEar;
  findEar.getPolygon(argv[1]);
  findEar.run();

  return 0;
}

void FindEar::getPolygon(const char *fileName)
{
  ifstream inFile(fileName);

  string line;
  double xx, yy;
  char comma;
  dpRing_t *ringTail, *newRing;
  while (getline(inFile, line)) {
    istringstream iss(line);
    iss >> xx >> comma >> yy;

    // cout << xx << " " << yy << "\n";
    newRing = new dpRing_t(xx, yy);
    _polygonIn._numPoints++;
    if (_polygonIn._ringStart == nullptr) {
        newRing->_next = newRing;
        newRing->_prev = newRing;
        _polygonIn._ringStart = ringTail = newRing;
    } else {
        newRing->_prev = ringTail;
        newRing->_next = ringTail->_next;
        ringTail->_next->_prev = newRing;
        ringTail->_next = newRing;
        ringTail = newRing;
    }
  }
  cout << "polygon size:" << _polygonIn._numPoints << "\n";
  inFile.close();
  // remove the last point if identical to start.
  // _polygonIn.checkLink();
  if (ringTail != _polygonIn._ringStart && ringTail->_pt == _polygonIn._ringStart->_pt) {
    _polygonIn.removeVetex(ringTail);
  }
  // _polygonIn.checkLink();
  return;
}

int FindEar::run()
{
  dpRing_t *prev, *curr, *next;
  _totalArea = 0.0;

  curr = _polygonIn._ringStart->_next;
  int limit = _polygonIn._numPoints;

  double prevArea = _polygonIn.getArea(), currArea = prevArea;
  cout << "Org area:" << prevArea << "\n";
  int i (0);
  while (_polygonIn._numPoints > 3 && currArea > THRESHOLD) {
    cout << "numPoints " << _polygonIn._numPoints << "\n";
    prev = _polygonIn._ringStart->_prev;
    curr = _polygonIn._ringStart;
    next = _polygonIn._ringStart->_next;
    i++;

    if (prev->_pt == curr->_pt) {
        printf("i: %d Remove identical node: (%g %g)\n", i, curr->_pt._x, curr->_pt._y);
        _polygonIn.removeVetex(curr);
        continue;
    } else if (next->_pt == curr->_pt) {
        printf("i: %d Remove identical node: (%g %g)\n", i, curr->_pt._x, curr->_pt._y);
        _polygonIn.removeVetex(curr);
        continue;
    }
    _polygonIn.checkLink();
    printf("cutline : (%g %g)(%g %g )(%g %g)\n",
      prev->_pt._x, prev->_pt._y, curr->_pt._x, curr->_pt._y, next->_pt._x, next->_pt._y);
    if (_polygonIn.lineIsInside(prev->_pt, next->_pt)) {
        // Take this solution.
        _triangles.push_back(dtriangle_t(prev->_pt, curr->_pt, next->_pt));
        double tArea = _triangles.back().getArea();
        cout << "T Area : " << tArea << "  prevArea: " << prevArea << " currArea: " << currArea - tArea << "\n";
        cout << "Area Gain : " << tArea << "\n";
        prevArea = currArea = currArea - tArea;
        _polygonIn.removeVetex(curr);
        _polygonIn.checkLink();
    } else {
        _polygonIn._ringStart = next;
    }
    cout << "i: " << i << "  currArea : " << currArea  << "\n";
    // Set a limit to avoid possible infinite loop caused by bad data or bad services.
    if (i > limit*30 || currArea == 0.0) break;
  }
  cout << "numPoints: " << _polygonIn._numPoints << "  currArea : " << currArea  << "\n";
  if (_polygonIn._numPoints == 3) {
     cout << "Add last rectangle\n";
     dpRing_t *curr = _polygonIn._ringStart;
     _triangles.push_back(dtriangle_t(curr->_prev->_pt, curr->_pt, curr->_next->_pt));
     double tArea = _triangles.back().getArea();
     cout << "T Area : " << tArea << "  prevArea: " << prevArea << " currArea: " << currArea - tArea << "\n";
  }

  for (dtriangle_t &t:_triangles) {
    cout << "[" << t._p1._x << "," << t._p1._y << "],["
         << t._p2._x << "," << t._p2._y << "],["
         << t._p3._x << "," << t._p3._y << "],\n";
    double tArea = t.getArea();
    // cout << "T Area :" << tArea << "\n";
    _totalArea += tArea;
  }
  cout << "Total Area : " << _totalArea << "\n";

  return 0;
}

// typedef bg::model::point<double, 2, bg::cs::cartesian> bgPoint_t;
typedef bg::model::d2::point_xy<double> bgPoint_t;
typedef bg::model::point<double, 2, bg::cs::cartesian> point_t;
typedef bg::model::multi_point<point_t> bgMpoint_t;
bool dpolygon_t::lineIsInside(const dpoint_t &p1, const dpoint_t &p2)
{
  // After experimenting, I found that the cover_by utility provided by Boost does not work.
#if 1 
  // boost geometry utility covered_by has bugs that it does not work properly for polygon covered_by polygon.
  // First check if the middle point is inside polygon and then check if there are intersections 
  // between line and each line on the polygon to make this run faster.
  
  bg::model::polygon<bgPoint_t> poly1;
  bgPoint_t bgPt;

  stringstream stream1;
  stream1  << "POLYGON((";

  dpRing_t *ringScan = _ringStart;
  do {
    stream1 << ringScan->_pt._x << " " << ringScan->_pt._y << ",";
    ringScan = ringScan->_next;
  } while (ringScan != _ringStart);
  stream1 << ringScan->_pt._x << " " << ringScan->_pt._y << "))";
  bg::read_wkt(stream1.str(), poly1);

  dpoint_t midPt((p1._x+p2._x)/2.0, (p1._y+p2._y)/2);
  stringstream stream2;
  stream2 << "POINT(" << midPt._x << " " << midPt._y << ")";
  bg::read_wkt(stream2.str(), bgPt);
  if (!bg::covered_by(bgPt, poly1)) return false;

  do {
    ringScan = ringScan->_next;
    dpoint_t p3 = ringScan->_pt, p4 = ringScan->_next->_pt;
    if (lineIntersect(p1, p2, p3, p4)) {
        return false;
    }
  } while (ringScan != _ringStart);
  return true;
#endif
}

bool lineIntersect(const dpoint_t &p1, dpoint_t const &p2, const dpoint_t &p3, const dpoint_t &p4) {
    if (p1 == p3 || p1 == p4 || p2 == p3 || p2 == p4)
        return false;

    double
    p1_p2_x = p1.distX(p2),
    p1_p2_y = p1.distY(p2),
    p3_p4_x = p3.distX(p4),
    p3_p4_y = p3.distY(p4),
    p1xp2y = p1._x*p2._y,
    p1yp2x = p1._y*p2._x,
    p3xp4y = p3._x*p4._y,
    p3yp4x = p3._y*p4._x,
    p1xp2y_p1yp2x = p1xp2y - p1yp2x,
    p3xp4y_p3yp4x = p3xp4y - p3yp4x,
    divideBy = (p1_p2_x * p3_p4_y) - (p1_p2_y * p3_p4_x);

    if (divideBy == 0) {
        return false;
    } else {
        double Px = ((p1xp2y - p1yp2x)*(p3_p4_x) - (p1_p2_x)*(p3xp4y - p3yp4x))/divideBy,
        Py = ((p1xp2y - p1yp2x)*(p3_p4_y) - (p1_p2_y)*(p3xp4y - p3yp4x))/divideBy;
        if (Px > fmin(p3._x, p4._x) && Px < fmax(p3._x, p4._x)
            && (Py > fmin(p3._y, p4._y) && Py < fmax(p3._y, p4._y))) {
                return true;
            }
    }

    return false;
}

/*********************************************************************************************
    Call boost to calculate area for a single triangle
*********************************************************************************************/
double dtriangle_t::getArea()
{
  bg::model::polygon<bg::model::d2::point_xy<double> > polyt;
  stringstream streamt;

  streamt << "POLYGON((";
  streamt << _p1._x << " " << _p1._y << ",";
  streamt << _p2._x << " " << _p2._y << ",";
  streamt << _p3._x << " " << _p3._y << ",";
  streamt << _p1._x << " " << _p1._y << "))";

  bg::read_wkt(streamt.str(), polyt);
  return fabs(bg::area(polyt));
}

double dpolygon_t::getArea()
{
  bg::model::polygon<bg::model::d2::point_xy<double> > poly1;

  stringstream stream1;
  stream1  << "POLYGON((";

  dpRing_t *ringScan = _ringStart;
  do {
    stream1 << ringScan->_pt._x << " " << ringScan->_pt._y << ",";
    ringScan = ringScan->_next;
  } while (ringScan != _ringStart);
  stream1 << ringScan->_pt._x << " " << ringScan->_pt._y << "))";
  cout << stream1.str() << "\n";

  bg::read_wkt(stream1.str(), poly1);
  return fabs(bg::area(poly1));
}

void dpolygon_t::checkLink()
{
   // forward link check
   dpRing_t *ringScan = _ringStart;
   cout << "Forward:\n";
   do {
     cout << "[" << ringScan->_pt._x << "," << ringScan->_pt._y << "],\n";
     ringScan = ringScan->_next;
   } while (ringScan != _ringStart);

   cout << "Backward:\n";
   do {
     cout << "[" << ringScan->_pt._x << "," << ringScan->_pt._y << "],\n";
     ringScan = ringScan->_prev;
   } while (ringScan != _ringStart);
   return;
}