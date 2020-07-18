#include "mathg.h"

int main()
{
	if (MathG::init(true))
	{
		bool ok;
		VectorG a(2, &ok);
		VectorG b(2, &ok);
		VectorG c(2, &ok);
		float adata[2] = { 0.3f, 0.3f };
		a.store(adata);
		float bdata[2] = { 0.2f, 0.2f };
		b.store(bdata);
		MathG::add(&a, &b, &c);
		float cdata[2];
		c.load(cdata);
		MathG::free();
	}
};