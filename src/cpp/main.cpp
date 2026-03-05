#include <iostream>
#include <cmath>
#include <vector>
#include <iomanip>
using namespace std;
//定义点的结构体
struct Point
{
    int x;
    int y;
};
//定义Rect结构体
struct Rect
{
    int id;
    int color;
    int width;
    int height;
    Point point;
};
//定义Armor类
class Armor
{
    private:
    //将rect作为Armor的成员变量，方便用其进行计算
        Rect rect;
    public:
        Armor(Rect RECT)
        {
            rect=RECT;
        }

        void Central_Point()
        {
            Point a;
            a.x = rect.point.x + rect.width/2;
            a.y = rect.point.y + rect.height/2;
            cout<<"("<<a.x<<","<<a.y<<")";           
        }

        void Diagonal()
        {
            float a = sqrt(rect.width*rect.width + rect.height*rect.height);
            cout<<"对角线长度："<<fixed<<setprecision(2)<<a<<endl;
        }

        void Armor_Point()
        {
            vector<Point> Points;
            int x = rect.point.x;
            int y = rect.point.y;
            int w = rect.width;
            int h = rect.height;

            Points.push_back({x,y});
            Points.push_back({x+w,y});
            Points.push_back({x+w,y+h});
            Points.push_back({x,y+h});
            
            for(int i = 0;i < 4; i++)
            {
                cout<<"("<<Points[i].x<<","<<Points[i].y<<")";
            }
            cout<<endl;
        }

        void Armor_Color()
        {
            if(rect.id==0)
            {
                cout<<"ID:0 颜色：蓝";
            }
            else
            {
                cout<<"ID:1 颜色：红";
            }
            printf("\n");
        }

};

int main()
{
    int id , color;
    cin>>id>>color;
    int x ,y , w, h;
    cin>>x>>y>>w>>h;

    Point point;
    point.x=x;
    point.y=y;

    Rect rect;
    rect.id=id;
    rect.color=color;
    rect.point=point;
    rect.width=w;
    rect.height=h;
    
    Armor armor(rect);
    armor.Armor_Color();
    armor.Central_Point();
    armor.Diagonal();
    armor.Armor_Point();

    return 0;

}