#include <iostream>
#include <Eigen/Dense>
 
using Eigen::MatrixXd;
 
int main(int argc, char *argv[])
{
	MatrixXd m(2,2);

	std::cout << "hello Eigen!" << std::endl;

	m(0,0) = 3;
	m(1,0) = 2.5;
	m(0,1) = -1;
	m(1,1) = m(1,0) + m(0,1);

	std::cout << m << std::endl;

	std::cout << "good bye Eigen!" << std::endl;

	return 0;
}
