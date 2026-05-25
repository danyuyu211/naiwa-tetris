#include <graphics.h>
#include <conio.h>

int main()
{
    initgraph(640, 480); // 创建 640x480 的窗口
    circle(320, 240, 100); // 画一个圆
    _getch(); // 按任意键退出
    closegraph();
    return 0;
}