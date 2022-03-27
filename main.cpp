
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <thread>
#include <cstdlib>
#include <ctime>

using Pos = int64_t;

constexpr int THR_NUM = 4; //

int* solve(const char* str, int64_t n) {
	int* vec = new int[n];

	std::vector<Pos> _stack; _stack.reserve(n / 2);

	int a = clock();
	for (int64_t i = 0; i < n; ++i) {
		if (str[i] == '(') {
			_stack.push_back(i);
		}
		else {
			if (!_stack.empty()) {
				int pos = _stack.back(); _stack.pop_back();
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
	int64_t rear = -1;
};



inline void push_back(Stack* _stack, Pos item) {
	++_stack->rear;
	_stack->_arr[_stack->rear] = item;
}

inline bool empty(Stack* _stack) {
	return _stack->rear == -1;
}

inline Pos back(Stack* _stack) {
	return _stack->_arr[_stack->rear];
}

inline void pop_back(Stack* _stack) {
	if (empty(_stack)) {
		std::cout << "pop_back error\n";
		return;
	}
	--_stack->rear;
}

inline void push_front(Stack* _stack, Pos item) {
	// shift
	for (int64_t i = _stack->rear; i >= 0; --i) {
		_stack->_arr[i + 1] = _stack->_arr[i];
	}
	// insert
	_stack->_arr[0] = item;
	++_stack->rear;
}

inline void pop_front(Stack* _stack) {
	// shift
	for (int64_t i = 0; i < _stack->rear; ++i) {
		_stack->_arr[i] = _stack->_arr[i + 1];
	}
	//
	--_stack->rear;
}

inline Pos front(Stack*& _stack) {
	return _stack->_arr[0];
}

inline int64_t size(Stack* _stack) {
	return _stack->rear + 1;
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
					(vec)[start + i] = x - 1 ;
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
			if (_stack[i]->_arr[j] > 0) {
				break;
			}
			idx++;
		}
		if (idx > -1) {
			for (int64_t j = idx; j >= 0; --j) {
				Pos before = back(_stack[0]); pop_back(_stack[0]);
				Pos now = _stack[i]->_arr[j];

				vec[before - 1] = -now - 1;
				vec[-now - 1] = before - 1;

			//	std::cout << " " << j <<  " " <<  - now - 1 << " " << before - 1 << "\n";
			}
			for (int64_t j = idx + 1; j < size(_stack[i]); ++j) {
				push_back(_stack[0], _stack[i]->_arr[j]);
			}
		}
		else {
			for (int64_t j = 0; j < size(_stack[i]); ++j) {
				push_back(_stack[0], _stack[i]->_arr[j]);
			}
		}
	}
}

Pos* solve_parallel(const char* str, int64_t n) {
	//std::cout << n << "\n";

	Pos* vec = new Pos[n];
	Stack* _stack[THR_NUM];
	std::thread thr[THR_NUM];

	for (int i = 0; i < THR_NUM; ++i) {
		_stack[i] = new Stack();
		if (i == 0) {
			_stack[i]->_arr = new Pos[n];
		}
		else {
			_stack[i]->_arr = new Pos[1 + (i == THR_NUM  - 1) ? (n - n / THR_NUM * i) : (n / THR_NUM * (i + 1) - n / THR_NUM * i)];
		}

		thr[i] = std::thread(solve_parallel_part1, vec, _stack[i],
			str + n / THR_NUM * i, n / THR_NUM * i, (i == THR_NUM  - 1) ? (n - n / THR_NUM * i) : (n / THR_NUM * (i + 1) - n / THR_NUM * i));
	}

	for (int i = 0; i < THR_NUM; ++i) {
		thr[i].join();
	}

	solve_parallel_part2(vec, _stack);

	for (int i = 0; i < THR_NUM; ++i) {
		delete[] _stack[i]->_arr;
		delete _stack[i];
	}

	return vec;
}


int main(void)
{
	std::string str = "((()))()()"; // must valid.
		
	for (int i = 0; i < 25; ++i) {
		str += str;
	}

	str = "((" + str + "))";

	std::cout << "init end\n";
	int a = clock();
	int* sol = solve(str.c_str(), str.size());
	int b = clock();
	std::cout << "total " << b - a << "ms\n";
	for (int64_t i = 0; i < str.size(); ++i) {
		std::cout << sol[i] << " "; break;
	}

	std::cout << "\n";
	delete[] sol;

	a = clock();
	Pos* sol2 = solve_parallel(str.c_str(), str.size());
	b = clock();
	std::cout << "total " << b - a << "ms\n";

	for (int64_t i = 0; i < str.size(); ++i) {
		std::cout << sol2[i] << " "; break;
	}

	delete[] sol2;
	std::cout << "\n";

	return 0;
}

