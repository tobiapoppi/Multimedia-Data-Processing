#include "canvas.h"
#include <stdbool.h>
#include "string.h"
#include <cstdlib>
#include <crtdbg.h>


/*
<type of object> <param_1> <param_2> ... <param_n>

rectangle 0 0 56 24 §
circle 70 10 4 @
circle 10 10 4 @
point 6 73 ù
line 4 18 76 12 *
*/

struct shape {
	int x_, y_;
	char style_;
	char* name_;

	shape(int x, int y, char style) : x_(x), y_(y), style_(style), name_(nullptr) { }
	
	virtual ~shape() {
		delete[] name_;
	}
	
	void set_name(const char* name) {
		delete[] name_;
		name_ = new char[strlen(name) + 1];
		strcpy(name_, name);
	}

	/*
	void shape_create_from_file(FILE* f) {
		fprintf(stderr, "ERROR: Shapes cannot be read from file.\n");
		exit(EXIT_FAILURE);
	}*/

	const char* name() const {
		return name_;
	}

	virtual void draw(canvas& c) {
		c.set(x_, y_, style_);
	}

};

struct point : public shape {
	point(int x, int y, char style) : shape(x, y, style) {}
	void draw(canvas& c) {
		c.set(x_, y_, style_);
	}
};

struct rectangle : public shape {
	int x1_, y1_;
	rectangle(int x0, int y0, int x1, int y1, char style) : shape(x0, y0, style), x1_(x1), y1_(y1) {}
	void draw(canvas& c) override {
		c.rectangle(x_, y_, x1_, y1_, style_);
	}
};

struct line : public shape {
	int x1_, y1_;
	line(int x0, int y0, int x1, int y1, char style) : shape(x0, y0, style), x1_(x1), y1_(y1) {}
	void draw(canvas& c) override {
		c.line(x_, y_, x1_, y1_, style_);
	}
};

struct circle : public shape {
	int r_;
	int *test_;

	circle(int xm, int ym, int r, char style) : shape(xm, ym, style), r_(r) {
		test_ = new int;
	}

	~circle() {
		delete test_;
	}

	void draw(canvas& c) override {
		c.circle(x_, y_, r_, style_);
	}
};
/*
void draw_file(FILE* filename, canvas* c) {
	char type[30];
	while (fscanf(filename, "%29s", type) == true) {
		if (strcmp(type, "point") == 0){
			int x, y;
			char ch;
			fscanf(filename, " %d%d %c", &x, &y, &ch);
			canvas_set(c, x, y, ch);
		}
		else if (strcmp(type, "rectangle") == 0) {
			int x0, y0, x1, y1;
			char ch;
			fscanf(filename, " %d%d%d%d %c", &x0, &y0, &x1, &y1, &ch);
			canvas_rectangle(c, x0, y0, x1, y1, ch);
		}
		else if (strcmp(type, "circle") == 0) {
			int xm, ym, r;
			char ch;
			fscanf(filename, " %d%d%d %c", &xm, &ym, &r, &ch);
			canvas_circle(c, xm, ym, r, ch);
		}
		else if (strcmp(type, "line") == 0) {
			int x0, y0, x1, y1;
			char ch;
			fscanf(filename, " %d%d%d%d %c", &x0, &y0, &x1, &y1, &ch);
			canvas_line(c, x0, y0, x1, y1, ch);
		}
	}
}*/

int main(void) {
	canvas c(80, 25);
	/*
	canvas_rectangle(c, 0, 0, 56, 24, '§');
	canvas_circle(c, 70, 10, 4, '@');
	canvas_circle(c, 10, 10, 4, '@');
	
	canvas_set(c, 6, 73, 'ù');

	canvas_line(c, 4, 18, 76, 12, '*');
	*/

	rectangle* r = new rectangle(0, 0, 56, 24, '§');
	point* p = new point(6, 73, 'ù');
	circle* cc = new circle(70, 10, 4, '@');
	line* l1 = new line(4, 18, 5, 65, '*');
	line* l2 = new line(4, 18, 76, 12, '*');

	r->set_name("my rect");
	p->set_name("my point");
	cc->set_name("my circle");
	l1->set_name("my 1st line");
	l2->set_name("my 2nd line");


	shape* arr[5] = { r, p, cc, l1, l2 };

	for (size_t i = 0; i < 5; ++i) {
		arr[i]->draw(c);
	}

	for (size_t i = 0; i < 5; ++i) {
		puts(arr[i]->name());
	}
	for (size_t i = 0; i < 5; ++i) {
		delete arr[i];
	}
	c.out(stdout);

	//I get a memory leak on canvas just because it is not yet out of scope. As soon as the program exits, canvas will be deleted.
	_CrtDumpMemoryLeaks(); 
}