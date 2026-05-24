#include <memory>
struct Widget {
    int id{0};
    double cache{0};
};
void demo_aliasing() {
    auto parent = std::make_shared<Widget>();
    parent->id = 42;
    // 与 parent 共享控制块，但 get() 指向 parent->cache
    std::shared_ptr<double> view(parent, &parent->cache);
    parent.reset();  // 若 view 还在，父对象因 view 的 strong 仍存活
    *view = 3.14;    // 改的是原 Widget 里的 cache
}  // 最后一个 shared_ptr（view）销毁 → Widget 整体释放
 