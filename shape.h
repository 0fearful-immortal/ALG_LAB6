#include <list>
#include <exception>

using std::list;

class ShapeException : public std::runtime_error // потому что исключительная ситуация возникает во время выполнения программы (выход за пределы)
{
public:
	ShapeException(const std::string& err) : runtime_error(err) {}
};

class OutOfScreen : public ShapeException
{
public:
	OutOfScreen() : ShapeException("Point is out of screen!\n") {}
};

class WrongParameters : public ShapeException
{
public:
	WrongParameters() : ShapeException("Wrong shape's parameters!\n") {}
};

class InvalidRotation: public ShapeException
{
public:
	InvalidRotation() : ShapeException("Cannot rotate already rotated shape!\n") {}
};

class InvalidReflection : public ShapeException
{
public:
	InvalidReflection() : ShapeException("Cannot reflect already reflected shape!\n") {}
};

class WrongAttachment : public ShapeException
{
public:
	WrongAttachment() : ShapeException("Invalid shapes attachment!\n") {}
};



//==1. Поддержка экрана в форме матрицы символов ==
char screen[YMAX][XMAX];
enum color { black = '*', white = '.' };
void screen_init()
{
	for (auto y = 0; y < YMAX; ++y)
		for (auto& x : screen[y])  x = white;
}
void screen_destroy()
{
	for (auto y = 0; y < YMAX; ++y)
		for (auto& x : screen[y])  x = black;
}
bool on_screen(int a, int b) // проверка попадания точки на экран
{
	return 0 <= a && a < XMAX && 0 <= b && b < YMAX;
}
void put_point(int a, int b)
{
	if (on_screen(a, b)) screen[b][a] = black;
}
void put_line(int x0, int y0, int x1, int y1)
/* Алгоритм Брезенхэма для прямой:
рисование отрезка прямой от (x0, y0) до (x1, y1).
Уравнение прямой: b(x–x0) + a(y–y0) = 0.
Минимизируется величина abs(eps), где eps = 2*(b(x–x0)) + a(y–y0).  */
{
	if (y0 > YMAX || y1 > YMAX || x0 > XMAX || x1 > XMAX) throw OutOfScreen();
	int dx = 1;
	int a = x1 - x0;   if (a < 0) dx = -1, a = -a;
	int dy = 1;
	int b = y1 - y0;   if (b < 0) dy = -1, b = -b;
	int two_a = 2 * a;
	int two_b = 2 * b;
	int xcrit = -b + two_a;
	int eps = 0;
	for (;;) { //Формирование прямой линии по точкам
		put_point(x0, y0);
		if (x0 == x1 && y0 == y1) break;
		if (eps <= xcrit) x0 += dx, eps += two_b;
		if (eps >= a || a < b) y0 += dy, eps -= two_a;
	}
}
void screen_clear() { screen_init(); } //Очистка экрана
void screen_refresh() // Обновление экрана
{
	for (int y = YMAX - 1; 0 <= y; --y) { // с верхней строки до нижней
		for (auto x : screen[y])                 // от левого столбца до правого
			std::cout << x;
		std::cout << '\n';
	}
}
//== 2. Библиотека фигур ==
struct shape {   // Виртуальный базовый класс «фигура»
	static list<shape*> shapes;       // Список фигур (один на все фигуры!)
	shape() { shapes.push_back(this); } //Фигура присоединяется к списку
	virtual void rotate_right() { /* пустая реализация по умолчанию */ }
	virtual void rotate_left() { /* пустая реализация по умолчанию */ }
	virtual void flip_vertically() {};
	virtual point north() const = 0;  //Точки для привязки
	virtual point south() const = 0;
	virtual point east() const = 0;
	virtual point west() const = 0;
	virtual point neast() const = 0;
	virtual point seast() const = 0;
	virtual point nwest() const = 0;
	virtual point swest() const = 0;
	virtual void draw() = 0;		//Рисование
	virtual void move(int, int) = 0;	//Перемещение
	virtual void resize(double) = 0;    	//Изменение размера
	virtual ~shape() { shapes.remove(this); } //Деструктор
};
list<shape*> shape::shapes;   // Размещение списка фигур


class rotatable : virtual public shape { 	//Фигуры, пригодные к повороту 
protected:
	enum class rotated { left = 1, no = 0, right = -1 };
	rotated state;           //Текущее состояние поворота
	rotated prev_rotation;
public:
	rotatable(rotated r = rotated::no) : state(r), prev_rotation(rotated::no) {}
	void rotate_left() {
		if (prev_rotation == rotated::left) throw InvalidRotation();
		state = rotated::left;
		prev_rotation = rotated::left;
	}
	//Повернуть влево
	void rotate_right() {
		if (prev_rotation == rotated::right) throw InvalidRotation();
		state = rotated::right;
		prev_rotation = rotated::right;
	}
	//Повернуть вправо
};

class reflectable : virtual public shape { 	//Фигуры, пригодные к зеркальному отражению
protected:
	bool hor, vert;         //Текущее состояние отражения
	bool prev_hor, prev_vert;
public:
	reflectable(bool h = false, bool v = false) : hor(h), vert(v), prev_hor(h), prev_vert(v) {}
	void flip_horisontally() {
		if (prev_hor) throw InvalidReflection();
		hor = !hor;
		prev_hor = true;
	}		//Отразить горизонтально
	void flip_vertically() {
		if (prev_vert) throw InvalidReflection();
		vert = !vert;
		prev_vert = true;
	} 	//Отразить вертикально
};

class line : public shape {        // ==== Прямая линия ====
	/* отрезок прямой ["w", "e"].
	   north( ) определяет точку «выше центра отрезка и так далеко
	   на север, как самая его северная точка», и т. п. */
protected:
	point w, e;

	bool all_points() const {
		return on_screen(w.x, w.y) && on_screen(e.x, e.y);
	}
public:
	line(point a, point b) : w(a), e(b) {
		if (!all_points()) {
			throw OutOfScreen();
		}
	}; //Произвольная линия (по двум точкам)
	line(point a, int L) : w(point(a.x + L - 1, a.y)), e(a) {
		if (!on_screen(a.x + L - 1, a.y)) throw OutOfScreen();
	}; //Горизонтальная линия
	point north() const { return point((w.x + e.x) / 2, e.y < w.y ? w.y : e.y); }
	point south() const { return point((w.x + e.x) / 2, e.y < w.y ? e.y : w.y); }
	point east() const { return point(e.x < w.x ? w.x : e.x, (w.y + e.y) / 2); }
	point west() const { return point(e.x < w.x ? e.x : w.x, (w.y + e.y) / 2); }
	point neast() const { return point(w.x < e.x ? e.x : w.x, e.y < w.y ? w.y : e.y); }
	point seast() const { return point(w.x < e.x ? e.x : w.x, e.y < w.y ? e.y : w.y); }
	point nwest() const { return point(w.x < e.x ? w.x : e.x, e.y < w.y ? w.y : e.y); }
	point swest() const { return point(w.x < e.x ? w.x : e.x, e.y < w.y ? e.y : w.y); }
	void move(int a, int b) { w.x += a; w.y += b; e.x += a; e.y += b; }
	void draw(){
		put_line(w, e);
	}
	void resize(double d) override               // Изменение длины линии в (d) раз
	{
		e.x = w.x + (e.x - w.x) * d; e.y = w.y + (e.y - w.y) * d;
	}
};
class rectangle : public rotatable {      // ==== Прямоугольник ====
	/* nw ------ n ------ ne
	   |		       |
	   |		       |
	   w	   c            e
	   |		       |
	   |		       |
	   sw ------- s ------ se */
protected:
	point sw, ne;
public:
	rectangle(point a, point b) : sw(a), ne(b) {
		if ((a.x == b.x) || (a.y == b.y)) throw WrongParameters();
		if (a.x < 0 || a.y < 0 || a.x > XMAX || a.y > YMAX || 
			b.x < 0 || b.y < 0 || b.x > XMAX || b.y > YMAX){
			std::cout << "Throwing OutOfScreen" << std::endl;
			throw OutOfScreen();
		}
	}
	point north() const { return point((sw.x + ne.x) / 2, ne.y); }
	point south() const { return point((sw.x + ne.x) / 2, sw.y); }
	point east() const { return point(ne.x, (sw.y + ne.y) / 2); }
	point west() const { return point(sw.x, (sw.y + ne.y) / 2); }
	point neast() const { return ne; }
	point seast() const { return point(ne.x, sw.y); }
	point nwest() const { return point(sw.x, ne.y); }
	point swest() const { return sw; }
	void rotate_right() override          // Поворот вправо относительно se
	{
		try {
			if (state == rotated::right) { throw InvalidRotation(); }

			int w = ne.x - sw.x, h = ne.y - sw.y;
			point old_sw = sw;
			point old_ne = ne;

			sw.x = ne.x - h * 2;
			ne.y = sw.y + w / 2;

			if (sw.x >= ne.x || sw.y >= ne.y) {
				sw = old_sw;
				ne = old_ne;
				throw WrongParameters();
			}
			if (!on_screen(sw.x, sw.y) || !on_screen(ne.x, ne.y) ||	!on_screen(ne.x, sw.y) || !on_screen(sw.x, ne.y)) throw OutOfScreen();

			state = rotated::right;
			rotatable::rotate_right();

		}
		catch (const ShapeException& err) {
			std::cerr << "Rectangle rotation error: " << err.what() << std::endl;
			throw;
		}
	}
	void rotate_left() override // Поворот влево относительно sw
	{
		try {
			if (state == rotated::left) { throw InvalidRotation(); }

			int w = ne.x - sw.x, h = ne.y - sw.y;
			point old_sw = sw;
			point old_ne = ne;

			ne.x = sw.x + h * 2;
			ne.y = sw.y + w / 2;

			if (sw.x >= ne.x || sw.y >= ne.y) {
				sw = old_sw;
				ne = old_ne;
				throw WrongParameters();
			}
			if (!on_screen(sw.x, sw.y) || !on_screen(ne.x, ne.y) ||	!on_screen(ne.x, sw.y) || !on_screen(sw.x, ne.y)) throw OutOfScreen();

			state = rotated::left;
			rotatable::rotate_left();

		}
		catch (const ShapeException& err) {
			std::cerr << "Rectangle rotation error: " << err.what() << std::endl;
			throw;
		}
	}
	void move(int a, int b)
	{
		point old_sw = sw;
		point old_ne = ne;

		sw.x += a; sw.y += b;
		ne.x += a; ne.y += b;

		if (!on_screen(sw.x, sw.y) || !on_screen(ne.x, ne.y) ||
			!on_screen(ne.x, sw.y) || !on_screen(sw.x, ne.y)) {
			sw = old_sw;
			ne = old_ne;
			throw OutOfScreen();
		}
	}
	void resize(double d)
	{
		try {
			if (d <= 0) { throw WrongParameters(); }

			point old_sw = sw;
			point old_ne = ne;

			ne.x = sw.x + (ne.x - sw.x) * d;
			ne.y = sw.y + (ne.y - sw.y) * d;

			if (sw.x >= ne.x || sw.y >= ne.y) {
				sw = old_sw;
				ne = old_ne;
				throw WrongParameters();
			}

			if (!on_screen(sw.x, sw.y) || !on_screen(ne.x, ne.y) ||
				!on_screen(ne.x, sw.y) || !on_screen(sw.x, ne.y)) {
				sw = old_sw;
				ne = old_ne;
				throw OutOfScreen();
			}

		}
		catch (const ShapeException& err) {
			std::cerr << "Rectangle resize error: " << err.what() << std::endl;
			throw;
		}
	}
	void draw()
	{
		put_line(nwest(), ne);   put_line(ne, seast());
		put_line(seast(), sw);   put_line(sw, nwest());
	}
};

class ErrorShape : public shape {
	point location;
public:
	ErrorShape(point p) : location(p) {}
	point north() const override { return location; }
	point south() const override { return location; }
	point east() const override { return location; }
	point west() const override { return location; }
	point neast() const override { return location; }
	point seast() const override { return location; }
	point nwest() const override { return location; }
	point swest() const override { return location; }
	void draw() override {
		put_line(location.x - 5, location.y, location.x + 5, location.y);
		put_line(location.x, location.y - 2, location.x, location.y + 2);
	}
	void move(int a, int b) override { location.x += a; location.y += b; }
	void resize(double) override {}
};

void shape_refresh()    // Перерисовка всех фигур на экране
{
	screen_clear();
	for (auto p : shape::shapes) {
		try {
			p->draw();
		}
		catch (const OutOfScreen& e) {
			point c((p->north().x + p->south().x) / 2, (p->north().y + p->south().y) / 2);
			ErrorShape err(c);
			err.draw();  // временно рисуем крестик
		}
	}
	screen_refresh();
}

void up(shape& p, const shape& q) // поместить фигуру p над фигурой q
{	//Это ОБЫЧНАЯ функция, не член класса! Динамическое связыва-ние!!
	point q_north = q.north();
	point p_south = p.south();

	int p_width = abs(p.seast().x - p.swest().x);    // ширина нижней грани p
	int q_width = abs(q.neast().x - q.nwest().x);    // ширина верхней грани q

	if (abs(p_width - q_width) > 30 && dynamic_cast<ErrorShape*>(&p) == nullptr && dynamic_cast<const ErrorShape*>(&q) == nullptr) {
		throw WrongAttachment();
	}

	p.move(q_north.x - p_south.x, q_north.y - p_south.y + 1);
}
