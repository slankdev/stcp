

# Timer処理を行う方法

stcp_timer__funcクラスを使う

## Sample Code

```
class func : public stcp_timer_func {
public:
    func(uint64_t ms) : stcp_timer_func(ms) {}
    void exec() override {
        printf("TIMER: 1sec\n");
    }
};

int main(int argc, char** argv)
{
	core::init(argc, argv);

    func f(1000);
    core::add_cyclic(&f);

	core::run();
}
```

