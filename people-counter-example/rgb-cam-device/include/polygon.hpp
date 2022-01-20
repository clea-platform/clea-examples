
#include <vector>




typedef struct {
    double x;
    double y;
} Point;

using Points    = std::vector<Point>;




class Polygon {
public:
    Polygon (uint zone_id, QString zone_name);
    Polygon (uint zone_id, QString zone_name, Points &vertices);
    ~Polygon () = default;
    
    void add_vertex (Point point);
    void reset_vertices (Points &new_vertices);
    bool contains (Point point);

    uint get_zone_id ();
    QString get_zone_name ();
    Points get_vertices ();

private:
    uint m_zone_id;
    QString m_zone_name;
    Points m_vertices;
};