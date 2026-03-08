// Пополнение и использование библиотеки фигур
#include <locale.h>
#include <iostream>
#include "screen.h"
#include "shape.h"
// ПРИМЕР ДОБАВКИ: дополнительный фрагмент – полуокружность
class h_circle : public rectangle, public reflectable {
public:
	h_circle(point a, int rd)
		: rectangle(point(a.x - rd, a.y), point(a.x + rd, a.y + rd * 0.7 + 1)) {
		if (!on_screen(sw.x, sw.y) || !on_screen(ne.x, ne.y) ||
			!on_screen(ne.x, sw.y) || !on_screen(sw.x, ne.y))
			throw OutOfScreen();
	}
	void draw();
	void flip_horisontally() {};   // Отразить горизонтально (пустая функция)
	void rotate_right() {}     // Повернуть вправо 
	void rotate_left() {}      // Повернуть влево
};
void h_circle::draw()   //Алгоритм Брезенхэма для окружностей
{  // (выдаются два сектора, указываемые значением reflected::vert)
	int x0 = (sw.x + ne.x) / 2, y0 = vert ? ne.y : sw.y;
	int radius = (ne.x - sw.x) / 2;
	int x = 0, y = radius, delta = 2 - 2 * radius, error = 0;
	while (y >= 0) {
		// Проверка каждой точки перед отрисовкой
		if (vert) {
			if (on_screen(x0 + x, y0 - y * 0.7) && on_screen(x0 - x, y0 - y * 0.7)) {
				put_point(x0 + x, y0 - y * 0.7);
				put_point(x0 - x, y0 - y * 0.7);
			}
			else {
				throw OutOfScreen();
			}
		}
		else {
			if (on_screen(x0 + x, y0 + y * 0.7) && on_screen(x0 - x, y0 + y * 0.7)) {
				put_point(x0 + x, y0 + y * 0.7);
				put_point(x0 - x, y0 + y * 0.7);
			}
			else {
				throw OutOfScreen();
			}
		}

		error = 2 * (delta + y) - 1;
		if (delta < 0 && error <= 0) { ++x; delta += 2 * x + 1; continue; }
		error = 2 * (delta - x) - 1;
		if (delta > 0 && error > 0) { --y; delta += 1 - 2 * y; continue; }
		++x; delta += 2 * (x - y);  --y;
	}
	//	rectangle::draw();
}
// ПРИМЕР ДОБАВКИ: дополнительная функция присоединения…
void down(shape& p, const shape& q) // поместить фигуру p под фигурой q
{
	point q_south = q.south();
	point p_north = p.north();

	int p_width = abs(p.neast().x - p.nwest().x);
	int q_width = abs(q.seast().x - q.swest().x);    

	if (abs(p_width - q_width) > 30) {
		throw WrongAttachment();
	}

	p.move(q_south.x - p_north.x, q_south.y - p_north.y - 1);
}
// Cборная пользовательская фигура – физиономия

void left(shape& p, const shape& q) {
	int dx = q.west().x - p.east().x - 1;
	int dy = q.west().y - p.east().y;

	// Проверка совместимости по высоте (аналогично up, но для вертикального выравнивания)
	int p_height = abs(p.north().y - p.south().y);
	int q_height = abs(q.north().y - q.south().y);
	if (abs(p_height - q_height) > 10)  // порог можно настроить
		throw WrongAttachment();

	point new_ne = p.neast();
	new_ne.x += dx; new_ne.y += dy;
	point new_sw = p.swest();
	new_sw.x += dx; new_sw.y += dy;

	if (!on_screen(new_ne.x, new_ne.y) || !on_screen(new_sw.x, new_sw.y) ||
		!on_screen(new_ne.x, new_sw.y) || !on_screen(new_sw.x, new_ne.y))
		throw OutOfScreen();

	// Если все проверки пройдены, выполняем перемещение
	p.move(dx, dy);
}

void right(shape& p, const shape& q) {
	int dx = q.east().x - p.west().x + 1;
	int dy = q.east().y - p.west().y;

	// Проверка совместимости по высоте
	int p_height = abs(p.north().y - p.south().y);
	int q_height = abs(q.north().y - q.south().y);
	if (abs(p_height - q_height) > 10)
		throw WrongAttachment();

	// Проверка попадания на экран после перемещения
	point new_ne = p.neast();
	new_ne.x += dx; new_ne.y += dy;
	point new_sw = p.swest();
	new_sw.x += dx; new_sw.y += dy;

	if (!on_screen(new_ne.x, new_ne.y) || !on_screen(new_sw.x, new_sw.y) ||
		!on_screen(new_ne.x, new_sw.y) || !on_screen(new_sw.x, new_ne.y))
		throw OutOfScreen();

	p.move(dx, dy);
}


class trapezoid : public reflectable, public rotatable {
protected:
	point nw, ne, se, sw;
	bool all_points() const {
		return on_screen(nw.x, nw.y) && on_screen(ne.x, ne.y) &&
			on_screen(se.x, se.y) && on_screen(sw.x, sw.y);
	}
public:
	trapezoid(point left_bottom, point right_top, int top_width) {
		if (left_bottom.x >= right_top.x || left_bottom.y >= right_top.y) {
			throw WrongParameters();
		}

		if (top_width <= 0 || top_width > (right_top.x - left_bottom.x)) {
			throw WrongParameters();
		}
		sw = left_bottom;
		se = point(right_top.x, left_bottom.y);
		int bottom_width = right_top.x - left_bottom.x;
		int left_offset = (bottom_width - top_width) / 2;
		nw = point(left_bottom.x + left_offset, right_top.y);
		ne = point(nw.x + top_width, right_top.y);

		if (nw.x == ne.x || sw.x == se.x || nw.y == sw.y || ne.y == se.y) {
			throw WrongParameters(); 
		}
		if (!on_screen(sw.x, sw.y) || !on_screen(se.x, se.y) ||
			!on_screen(nw.x, nw.y) || !on_screen(ne.x, ne.y)) {
			throw OutOfScreen();
		}
	}
	point north() const { return point((nw.x + ne.x) / 2, (nw.y + ne.y) / 2); }
	point south() const { return point((sw.x + se.x) / 2, (sw.y + se.y) / 2); }
	point east() const { return point((se.x + ne.x) / 2, (se.y + ne.y) / 2); }
	point west() const { return point((nw.x + sw.x) / 2, (nw.y + sw.y) / 2); }
	point neast() const { return ne; }
	point seast() const { return se; }
	point nwest() const { return nw; }
	point swest() const { return sw; }
	void draw() {
		try {
			put_line(nw, ne); put_line(ne, se); put_line(se, sw); put_line(sw, nw);
		}catch(const OutOfScreen& err) { std::cerr << "Trapezoid drawing error : " << err.what() << std::endl; }
	}
	void move(int a, int b) {
		nw.x += a; nw.y += b; ne.x += a; ne.y += b; sw.x += a; sw.y += b; se.x += a; se.y += b;
		if (!all_points()) {
			nw.x -= a; nw.y -= b; ne.x -= a; ne.y -= b; sw.x -= a; sw.y -= b; se.x -= a; se.y -= b;
			throw OutOfScreen();
		}
	}
	void resize(double d) {
		if (d <= 0) throw WrongParameters();
		point c(
			(nw.x + ne.x + se.x + sw.x) / 4,
			(nw.y + ne.y + se.y + sw.y) / 4
		);
		point old_nw = nw, old_ne = ne, old_sw = sw, old_se = se;

		nw.x = c.x + (nw.x - c.x) * d;
		nw.y = c.y + (nw.y - c.y) * d;

		ne.x = c.x + (ne.x - c.x) * d;
		ne.y = c.y + (ne.y - c.y) * d;

		se.x = c.x + (se.x - c.x) * d;
		se.y = c.y + (se.y - c.y) * d;

		sw.x = c.x + (sw.x - c.x) * d;
		sw.y = c.y + (sw.y - c.y) * d;
		
		if (!all_points()) {
			nw = old_nw; ne = old_ne; sw = old_sw; se = old_se;
			throw OutOfScreen();
		}

		if (abs(ne.x - nw.x) == 0 || abs(se.x - sw.x) == 0) {
			nw = old_nw; ne = old_ne; sw = old_sw; se = old_se;
			throw WrongParameters();
		}
	}

	void resize(int d) {
		if (d <= 0) throw WrongParameters();
		resize((double)d);
	}

	void rotate_left() {
		point new_nw, new_ne, new_sw, new_se;
		new_sw = rotate_point(nw, rotated::left);
		new_nw = rotate_point(ne, rotated::left);
		new_ne = rotate_point(se, rotated::left);
		new_se = rotate_point(sw, rotated::left);
		point old_nw = nw, old_ne = ne, old_sw = sw, old_se = se;
		nw = new_nw; ne = new_ne;
		sw = new_sw; se = new_se;
		try {
			state = rotated::left;
			rotatable::rotate_left();
		}catch(const InvalidRotation& err) {
			std::cerr << "Shape's rotation error: " << err.what() << std::endl;
			nw = old_nw; ne = old_ne; sw = old_sw; se = old_se;
			throw;
		}
	}
	void rotate_right() {
		point new_nw, new_ne, new_sw, new_se;
		new_ne = rotate_point(nw, rotated::right);
		new_se = rotate_point(ne, rotated::right);
		new_sw = rotate_point(se, rotated::right);
		new_nw = rotate_point(sw, rotated::right);
		point old_nw = nw, old_ne = ne, old_sw = sw, old_se = se;
		nw = new_nw; ne = new_ne;
		sw = new_sw; se = new_se;
		try {
			state = rotated::right;
			rotatable::rotate_right();
		}
		catch (const InvalidRotation& err) {
			std::cerr << "Shape's rotation error: " << err.what() << std::endl;
			nw = old_nw; ne = old_ne; sw = old_sw; se = old_se;
			throw;
		}
	}
	void flip_horisontally() {
		int xc = north().x;
		point new_nw, new_ne, new_se, new_sw;
		new_ne = point(2 * xc - nw.x, nw.y);
		new_nw = point(2 * xc - ne.x, ne.y);
		new_sw = point(2 * xc - se.x, se.y);
		new_se = point(2 * xc - sw.x, sw.y);
		point old_nw = nw, old_ne = ne, old_sw = sw, old_se = se;

		nw = new_nw; ne = new_ne; sw = new_sw; se = new_se;
		
		try {
			hor = !hor;
			reflectable::flip_horisontally();
		}
		catch (const InvalidReflection& err) {
			std::cerr << "Shape's reflection error: " << err.what() << std::endl;
			nw = old_nw; ne = old_ne; sw = old_sw; se = old_se;
			throw;
		}
	}

	void flip_vertically() override{
		int diff = north().y - south().y;
		point new_nw, new_ne, new_sw, new_se;
		new_sw = point(nw.x, nw.y - diff); new_se = point(ne.x, ne.y - diff);
		new_nw = point(sw.x, sw.y + diff); new_ne = point(se.x, se.y + diff);
		point old_nw = nw, old_ne = ne, old_sw = sw, old_se = se;

		sw = new_sw; se = new_se; nw = new_nw; ne = new_ne;
		
		try {
			vert = !vert;
			reflectable::flip_horisontally();
		}
		catch (const InvalidReflection& err) {
			std::cerr << "Shape's reflection error: " << err.what() << std::endl;
			nw = old_nw; ne = old_ne; sw = old_sw; se = old_se;
			throw;
		}
	}

	trapezoid(const trapezoid& other) : nw(other.nw), ne(other.ne), se(other.se), sw(other.sw) {
		int height = south().y - north().y;

		point old_nw = nw, old_ne = ne, old_sw = sw, old_se = se;

		move(0, height - 3);

		if (!all_points()) {
			nw = old_nw; ne = old_ne; sw = old_sw; se = old_se;
			throw OutOfScreen();
		}
	}
private:
	trapezoid& operator =  (const trapezoid&) = delete;

	point rotate_point(point old_point, rotated r) {
		int dir = static_cast<int>(r);
		int old_x = old_point.x; int old_y = old_point.y;
		int xc = north().x; int yc = west().y;
		int new_x = -1 * (old_y - yc) * dir; int new_y = (old_x - xc) * dir;
		return point(new_x + xc, new_y + yc);
	}
};


class myshape : public rectangle {   // Моя фигура ЯВЛЯЕТСЯ
	int w, h;                        //        прямоугольником
	line l_eye;    // левый глаз – моя фигура СОДЕРЖИТ линию
	line r_eye;   // правый глаз
	line mouth;  // рот

	

public:
	myshape(point, point);
	void draw();
	void move(int, int);
	void resize(double r);
	void rotate_left();
	void rotate_right();
	bool all_points() const
	{
		return on_screen(swest().x, swest().y) &&
			on_screen(seast().x, seast().y) &&
			on_screen(nwest().x, nwest().y) &&
			on_screen(neast().x, neast().y);
	}
};

myshape::myshape(point a, point b)
	: rectangle(a, b),
	w(neast().x - swest().x + 1), 
	h(neast().y - swest().y + 1), 
	l_eye(point(swest().x + 2, swest().y + h * 3 / 4), 2),
	r_eye(point(swest().x + w - 4, swest().y + h * 3 / 4), 2),
	mouth(point(swest().x + 2, swest().y + h / 4), w - 4)
{
	try {
		if (!all_points()) {
			throw OutOfScreen();
		}

		if (w <= 4 || h <= 4) {
			throw WrongParameters();
		}
	}
	catch (const ShapeException& err) {
		std::cerr << "Myshape creating error: " << err.what() << std::endl;
		throw;
	}
}

void myshape::draw()
{
	try {
		rectangle::draw();
		int a = (swest().x + neast().x) / 2;
		int b = (swest().y + neast().y) / 2;

		if (!on_screen(a, b)) {
			throw OutOfScreen();
		}
		put_point(point(a, b));
	}
	catch (const OutOfScreen& err) {
		std::cerr << "Myshape drawing error: " << err.what() << std::endl;
		throw;
	}
}

void myshape::move(int a, int b)
{
	point old_sw = swest();
	point old_ne = neast();
	

	try {
		rectangle::move(a, b);

		if (!all_points()) {
			rectangle::move(-a, -b);
			throw OutOfScreen();
		}

		l_eye.move(a, b);
		r_eye.move(a, b);
		mouth.move(a, b);

	}
	catch (const OutOfScreen& err) {
		std::cerr << "Myshape move error: " << err.what() << std::endl;
		throw;
	}
}

void myshape::resize(double r)
{
	try {
		if (r <= 0) {
			throw WrongParameters();
		}

		int old_w = w;
		int old_h = h;
		point old_sw = swest();

		rectangle::resize(r);

		w = neast().x - swest().x + 1;
		h = neast().y - swest().y + 1;

		rectangle::move(old_w * (1 - r) * 0.5, old_h * (1 - r) * 0.5);

		if (!all_points()) {
			rectangle::resize(1.0 / r);
			rectangle::move(-(old_w * (1 - r) * 0.5), -(old_h * (1 - r) * 0.5));
			w = old_w;
			h = old_h;
			throw OutOfScreen();
		}

		l_eye = line(point(swest().x + 2, swest().y + h * 3 / 4), 2);
		r_eye = line(point(swest().x + w - 4, swest().y + h * 3 / 4), 2);
		mouth = line(point(swest().x + 2, swest().y + h / 4), w - 4);

	}
	catch (const ShapeException& err) {
		std::cerr << "Myshape resize error: " << err.what() << std::endl;
		throw;
	}
}

void myshape::rotate_left()
{
	try {
		if (state == rotated::left) {
			throw InvalidRotation();
		}

		rectangle::rotate_left();

		w = neast().x - swest().x + 1;
		h = neast().y - swest().y + 1;

		l_eye = line(point(swest().x + 2, swest().y + h * 3 / 4), 2);
		r_eye = line(point(swest().x + w - 4, swest().y + h * 3 / 4), 2);
		mouth = line(point(swest().x + 2, swest().y + h / 4), w - 4);

	}
	catch (const InvalidRotation& err) {
		std::cerr << "Myshape rotation error: " << err.what() << std::endl;
		throw;
	}
}

void myshape::rotate_right()
{
	try {
		if (state == rotated::right) {
			throw InvalidRotation();
		}

		rectangle::rotate_right();

		w = neast().x - swest().x + 1;
		h = neast().y - swest().y + 1;

		l_eye = line(point(swest().x + 2, swest().y + h * 3 / 4), 2);
		r_eye = line(point(swest().x + w - 4, swest().y + h * 3 / 4), 2);
		mouth = line(point(swest().x + 2, swest().y + h / 4), w - 4);

	}
	catch (const InvalidRotation& err) {
		std::cerr << "myshape rotation error: " << err.what() << std::endl;
		throw;
	}
}



int main()
{
	setlocale(LC_ALL, "Rus");
    screen_init();
    //== 1. Объявление набора фигур ==
    shape* hat = nullptr;
    try {
        hat = new rectangle(point(0, 0), point(10, 5));
    } catch (const ShapeException& err) {
        hat = new ErrorShape(point(5, 2));  // центр прямоугольника
        std::cerr << "Hat drawing error: " << err.what() << std::endl;
    } 

    shape* brim = nullptr;
    try {
        brim = new line(point(20, 25), 17);
    } catch(const OutOfScreen& err) {
        brim = new ErrorShape(point(28, 25)); // центр линии
        std::cerr << "Brim drawing error: " << err.what() << std::endl;
    }

    shape* face = nullptr;
    try {
        face = new myshape(point(15, 10), point(27, 18));
    }
    catch (const OutOfScreen& err) {
        face = new ErrorShape(point(21, 14)); // центр лица
        std::cerr << "Face creating error: " << err.what() << std::endl;
    }
    catch (const WrongParameters& err) {
        face = new ErrorShape(point(21, 14));
        std::cerr << "Face creating error: " << err.what() << std::endl;
    }
    catch (const ShapeException& err) {
        face = new ErrorShape(point(21, 14));
        std::cerr << "Face creating error: " << err.what() << std::endl;
    }
    catch (const InvalidRotation& err) {
        face = new ErrorShape(point(21, 14));
        std::cerr << "Face creating error: " << err.what() << std::endl;
    }
    
    shape* beard = nullptr;
    try {
        beard = new h_circle(point(40, 10), 5);
    }
    catch (const OutOfScreen& err) {
        beard = new ErrorShape(point(40, 10)); // центр полукруга
        std::cerr << "Beard drawing error: " << err.what() << std::endl;
    }

    shape* l_ear = nullptr;
    try {
        l_ear = new trapezoid(point(7, 40), point(108, 45), 5);
    }
    catch (const OutOfScreen& err) {
        l_ear = new ErrorShape(point(13, 42)); // примерный центр левого уха
        std::cerr << "Left ear drawing error: " << err.what() << std::endl;
    }
    catch (const WrongParameters& err) {
        l_ear = new ErrorShape(point(13, 42));
        std::cerr << "Left ear drawing error: " << err.what() << std::endl;
    } catch (const ShapeException& err) {
		l_ear = new ErrorShape(point(13, 42));
		std::cerr << "Face creating error: " << err.what() << std::endl;
	}

    shape* r_ear = nullptr;
    try {
        trapezoid* t = dynamic_cast<trapezoid*>(l_ear);
        if (!t) throw std::bad_cast();
        r_ear = new trapezoid(*t);
    }
    catch (const OutOfScreen& err) {
        r_ear = new ErrorShape(point(13, 42)); // используем те же координаты
        std::cerr << "Right ear drawing error: " << err.what() << std::endl;
    }
    catch (const std::bad_cast& err) {
        r_ear = new ErrorShape(point(13, 42));
        std::cerr << "Right ear drawing error: Invalid cast" << std::endl;
    }

    shape* tie = nullptr;
    try {
        tie = new trapezoid(point(37, 38), point(45, 45), 2);
    } catch (const OutOfScreen& err) {
        tie = new ErrorShape(point(41, 41)); // центр галстука
        std::cerr << "Tie drawing error: " << err.what() << std::endl;
    }
	catch (const WrongParameters& err) {
		tie = new ErrorShape(point(41, 41));
		std::cerr << "Tie drawing error: " << err.what() << std::endl;
	}


	shape_refresh();
	std::cout << "=== Generated... ===\n";
	std::cin.get(); //Смотреть исходный набор
	//== 2. Подготовка к сборке ==

	//== 2. Подготовка к сборке ==
	try { hat->rotate_right(); }
	catch (const ShapeException& err) { std::cerr << "Hat operation error: " << err.what() << std::endl;	}

	try { l_ear->resize(0.3); }
	catch (const ShapeException& err) { std::cerr << "Left ear operation error: " << err.what() << std::endl; }

	try { l_ear->rotate_left(); }
	catch (const ShapeException& err) { std::cerr << "Left ear operation error: " << err.what() << std::endl; }

	try { r_ear->resize(0.3); }
	catch (const ShapeException& err) { std::cerr << "Right ear operation error: " << err.what() << std::endl; }

	try { tie->flip_vertically(); }
	catch (const ShapeException& err) { std::cerr << "Tie operation error: " << err.what() << std::endl;	}

	try { r_ear->rotate_right(); }
	catch (const ShapeException& err) { std::cerr << "Right ear operation error: " << err.what() << std::endl; }

	try { brim->resize(2.0); }
	catch (const ShapeException& err) { std::cerr << "Brim operation error: " << err.what() << std::endl; }

	try { face->resize(1.2); }
	catch (const ShapeException& err) { std::cerr << "Face operation error: " << err.what() << std::endl; }

	try { beard->flip_vertically(); }
	catch (const ShapeException& err) { std::cerr << "Beard operation error: " << err.what() << std::endl; }

	try { beard->resize(1.2); }
	catch (const ShapeException& err) { std::cerr << "Beard operation error: " << err.what() << std::endl; }

	try { tie->resize(0.5); }
	catch (const ShapeException& err) { std::cerr << "Tie operation error: " << err.what() << std::endl; }

	shape_refresh();
	std::cout << "=== Prepared... ===\n";
	std::cin.get();
	//== 3. Сборка изображения ==
	//	face.move(0, -10); // Лицо – в исходное положение (если нужно!)
	try {
		up(*brim, *face);
	} catch (const ShapeException& err) { std::cerr << "Invalid shapes attachment: " << err.what() << std::endl; }
	try {
		down(*tie, *face);
	}catch (const ShapeException& err) { std::cerr << "Invalid shapes attachment: " << err.what() << std::endl; }
	try {
		up(*hat, *brim);
	}catch (const ShapeException& err) { std::cerr << "Invalid shapes attachment: " << err.what() << std::endl; }
	try {
		down(*beard, *face);
	}catch (const ShapeException& err) { std::cerr << "Invalid shapes attachment: " << err.what() << std::endl; }
	try {
		left(*l_ear, *face);
	}
	catch (const ShapeException& e) {
		delete l_ear;
		l_ear = new ErrorShape(point(13, 42));
		l_ear->rotate_left();
		left(*l_ear, *face);
		std::cerr << "Error in left: " << e.what();
	}
	catch (const std::exception& e) {
		std::cerr << "Unexpected error in left: " << e.what();
	}
	
	try {
		right(*r_ear, *face);
	}
	catch (const ShapeException& e) {
		delete r_ear;
		r_ear = new ErrorShape(point(13, 42));
		r_ear->rotate_right();
		right(*r_ear, *face);
		std::cerr << "Error in left: " << e.what();
	}
	catch (const std::exception& e) {
		std::cerr << "Unexpected error in left: " << e.what();
	}
	shape_refresh();
	std::cout << "=== Ready! ===\n";
	std::cin.get();       //Смотреть результат
	screen_destroy();
	return 0;
}

