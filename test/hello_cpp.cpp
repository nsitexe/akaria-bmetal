#include <iostream>

class hello {
public:
	hello() {
		std::cout << "hello::hello(): " << this << std::endl;

		val_con = 200;
	}

	~hello() {
		std::cout << "~hello::hello(): " << this << std::endl;
	}

	int func(int a) {
		std::cout << "hello::func(" << a << "): " << this << std::endl;
		std::cout << "  val_ini: " << val_ini << ": " << this << std::endl;
		std::cout << "  val_con: " << val_con << ": " << this << std::endl;

		val_ini = a;
		val_con = a;

		std::cout << "  val_ini: " << val_ini << ": " << this << std::endl;
		std::cout << "  val_con: " << val_con << ": " << this << std::endl;

		return a * 10;
	}

private:
	int val_ini = 100;
	int val_con;
};

hello h_global;

int main(int argc, char *argv[], char *envp[])
{
	std::cout << "hello world!" << std::endl;
	
	{
		hello h_local;

		std::cout << "h_global: " << &h_global << std::endl;
		std::cout << "h_local : " << &h_local << std::endl;

		h_global.func(10);
		h_local.func(20);
	}

	std::cout << "good bye world!" << std::endl;

	return 0;
}
