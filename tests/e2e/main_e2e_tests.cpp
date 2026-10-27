# include "e2eTests.hpp"

int main()
{
	e2eTests	t;

	t.run_all();
	if (t.getFailed() == 0)
		return (0);
	return (1);
}
