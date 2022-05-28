

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <thread>
#include <cstdlib>
#include <ctime>


class _Pos {
public:
	int64_t x;
public:
	_Pos(int64_t x = 0) : x(x) { }
	operator int64_t() { return x; }
};

using Pos = _Pos;

constexpr int THR_NUM = 4; //

Pos* solve(const char* str, int64_t n) {
	Pos* vec = (Pos*)calloc(n, sizeof(Pos));

	std::vector<Pos> _stack; _stack.reserve(n / 2);

	int a = clock();
	for (int64_t i = 0; i < n; ++i) {
		if (str[i] == '(') {
			_stack.push_back(i);
		}
		else {
			if (!_stack.empty()) {
				Pos pos = _stack.back(); _stack.pop_back();
				vec[i] = pos;
				vec[pos] = i;
			}
			// else { not valid }
		}
	}
	std::cout << clock() - a << "ms\n";
	return vec;
}


struct Stack {
	Pos* _arr = nullptr;
	Pos* start = nullptr;
	Pos* rear = nullptr;
};

inline void init(Stack* _stack, Pos* _arr, int n) {
	_stack->_arr = _arr;
	_stack->start = _arr + n;
	_stack->rear = _arr + n;
}

inline void push_back(Stack* _stack, Pos item) {
	*(_stack->rear) = item;
	++_stack->rear;
}

inline bool empty(Stack* _stack) {
	return _stack->rear == _stack->start;
}

inline Pos back(Stack* _stack) {
	// empty->err..
	return *(_stack->rear - 1);
}

inline void pop_back(Stack* _stack) {
	if (empty(_stack)) {
		std::cout << "pop_back error\n";
		return;
	}
	--_stack->rear;
}

inline void push_front(Stack* _stack, Pos item) {
	_stack->start--;
	*_stack->start = item;
}


inline Pos front(Stack* _stack) {
	return _stack->start[0];
}

inline int64_t size(Stack* _stack) {
	return _stack->rear - _stack->start;
}

inline void print(Stack* _stack) {
	for (int64_t i = 0; i < size(_stack); ++i) {
		std::cout << _stack->_arr[i] << " ";
	}
	std::cout << "\n";
}

void solve_parallel_part1(Pos* vec, Stack* _stack,
	const char* str, int64_t start, int64_t n)
{

	int a = clock();

	for (int64_t i = 0; i < n; ++i) {
		if (str[i] == '(') {
			push_back(_stack, (start + i + 1));
		}
		else {
			if (!empty(_stack)) {
				Pos x = back(_stack);
				if (x > 0) {
					(vec)[start + i] = x - 1;
					(vec)[x - 1] = start + i;

					pop_back(_stack);
				}
				else {
					push_front(_stack, -(start + i + 1));
				}
			}
			else {
				push_back(_stack, -(start + i + 1));
			}
		}
	}

	int b = clock();
	std::cout << b - a << "ms\n";
}

void solve_parallel_part2(Pos* vec, Stack* _stack[THR_NUM]) {

	for (int i = 1; i < THR_NUM; ++i) {
		int64_t idx = -1;
		for (int64_t j = 0; j < size(_stack[i]); ++j) {
			if (_stack[i]->start[j] > 0) {
				break;
			}
			idx++;
		}
		if (idx > -1) {
			for (int64_t j = idx; j >= 0; --j) {
				Pos before = back(_stack[0]); pop_back(_stack[0]);
				Pos now = _stack[i]->start[j];

				vec[before - 1] = -now - 1;
				vec[-now - 1] = before - 1;

				//	std::cout << " " << j <<  " " <<  - now - 1 << " " << before - 1 << "\n";
			}
			for (int64_t j = idx + 1; j < size(_stack[i]); ++j) {
				push_back(_stack[0], _stack[i]->start[j]);
			}
		}
		else {
			for (int64_t j = 0; j < size(_stack[i]); ++j) {
				push_back(_stack[0], _stack[i]->start[j]);
			}
		}
	}
}

Pos* solve_parallel(const char* str, uint64_t n) {
	//std::cout << n << "\n";
	int _a = clock();
	Pos* vec = (Pos*)calloc(n, sizeof(Pos));
	Stack* _stack[THR_NUM];
	std::thread thr[THR_NUM];

	for (int i = 0; i < THR_NUM; ++i) {
		_stack[i] = new Stack();
		if (i == 0) {
			init(_stack[i], (Pos*)calloc(2 + 2 * n, sizeof(Pos)), n);
		}
		else {
			int64_t sz = 1 + (i == THR_NUM - 1) ? (n - n / THR_NUM * i) : (n / THR_NUM * (i + 1) - n / THR_NUM * i);
			init(_stack[i], (Pos*)calloc(2 * sz, sizeof(Pos)), sz);
		}
	}
	std::cout << "chk.. " << clock() - _a << "ms\n";

	for (int i = 0; i < THR_NUM; ++i) {
		thr[i] = std::thread(solve_parallel_part1, vec, _stack[i],
			str + n / THR_NUM * i, n / THR_NUM * i, (i == THR_NUM - 1) ? (n - n / THR_NUM * i) : (n / THR_NUM * (i + 1) - n / THR_NUM * i));
	}

	for (int i = 0; i < THR_NUM; ++i) {
		thr[i].join();
	}

	int a = clock();
	solve_parallel_part2(vec, _stack);
	int b = clock();
	std::cout << "merge " << b - a << "ms\n";
	for (int i = 0; i < THR_NUM; ++i) {
		free(_stack[i]->_arr);
		delete _stack[i];
	}
	int _b = clock();
	std::cout << _b - _a << "ms\n";
	return vec;
}


int main(void)
{
	std::string str = "((()))()()"; // must valid.

	for (int i = 0; i < 23; ++i) {
		str += str;
	}

	str = "((" + str + "))";

	std::cout << "init end\n";
	int a = clock();
	Pos* sol = solve(str.c_str(), str.size());
	int b = clock();
	std::cout << "total " << b - a << "ms\n";
	for (int64_t i = 0; i < str.size(); ++i) {
		std::cout << sol[i] << " "; break;
	}

	std::cout << "\n";
	free(sol);

	a = clock();
	Pos* sol2 = solve_parallel(str.c_str(), str.size());
	b = clock();
	std::cout << "total " << b - a << "ms\n";

	for (int64_t i = 0; i < str.size(); ++i) {
		std::cout << sol2[i] << " "; break;
	}

	free(sol2);
	std::cout << "\n";

	return 0;
}

