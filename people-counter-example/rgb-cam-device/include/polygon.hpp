
#include <vector>




typedef struct {
    double x;
    double y;
} Point;

using Points    = std::vector<Point>;




class Polygon {
public:
    Polygon () = default;
    Polygon (Points &vertices);
    ~Polygon () = default;
    
    void add_vertex (Point point);
    void reset_vertices (Points &new_vertices);
    bool contains (Point point);

    Points get_vertices () {return m_vertices;}

private:
    Points m_vertices;
};