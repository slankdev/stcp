
# ユーザアプリケーションの使用方法

## Sample Code

```
static int user_main(stcp_thrd_arg* arg) {
	/*
	 * int argc    = arg->argc;
	 * char** argv = arg->argv;
	 */
    return 0;
}

int main(int argc, char** argv)
{
    core::init(argc, argv);
    core::set_hw_addr(0x00, 0x11 , 0x22 , 0x33 , 0x44 , 0x55);
    core::set_ip_addr(192, 168, 222, 10, 24);
    core::set_default_gw(192, 168, 222, 1, 0);

	core::set_app(user_main, NULL);
    core::run();
}
```


