#include <iostream>
#include <vector>
#include <set>
#include <bitset>
#include <functional>
#include <cmath>
#include <cstdlib>
#include <chrono>
#include <array>
#include <future>
#include <thread>

constexpr uint8_t BOARD_SIZE{9};
constexpr uint8_t NUM_STATES{9};

struct Vec2 {
	int x;
	int y;

	Vec2 operator/(int mult) const {
		return Vec2{x / mult, y / mult};
	}
};

struct Rule {
	std::function<std::vector<Vec2>(const Vec2&)> get_cells;
	std::function<bool(const std::vector<uint8_t>&)> is_valid;
};

using state_set = std::bitset<NUM_STATES>;

const Rule rules[] = {
	// Column
	{
		[](const Vec2& pos) -> std::vector<Vec2> {
			std::vector<Vec2> cells{};
			for (int i{0}; i < BOARD_SIZE; i++) {
				if(i != pos.y) cells.push_back(Vec2{pos.x, i});
			}

			return cells;
		},
		[](const std::vector<uint8_t>& cell_vals) -> bool {
			state_set states{};

			for (auto& val : cell_vals) {
				states.set(val);
			}

			return states.count() == BOARD_SIZE;
		}
	},
	// Row
	{
		[](const Vec2& pos) -> std::vector<Vec2> {
			std::vector<Vec2> cells{};
			for (int i{0}; i < BOARD_SIZE; i++) {
				if(i != pos.x) cells.push_back(Vec2{i, pos.y});
			}

			return cells;
		},
		[](const std::vector<uint8_t>& cell_vals) -> bool {
			state_set states{};

			for (auto& val : cell_vals) {
				states.set(val);
			}

			return states.count() == BOARD_SIZE;
		}
	},
	//Squares
	{
		[](const Vec2& pos) -> std::vector<Vec2> {
			int square_size{(int)ceil(sqrt(BOARD_SIZE))};
			Vec2 square_pos = pos / square_size;
			std::vector<Vec2> cells{};
			for (int i{0}; i < square_size; i++) {
				for (int j{0}; j < square_size; j++) {
					int x = square_pos.x * square_size + i;
					int y = square_pos.y * square_size + j;
					if (x != pos.x || y != pos.y) cells.push_back(Vec2{x, y});
				}
			}

			return cells;
		},
		[](const std::vector<uint8_t>& cell_vals) -> bool {
			state_set states{};

			for (auto& val : cell_vals) {
				states.set(val);
			}

			return states.count() == BOARD_SIZE;
		}
	}
};

struct Cell;

std::ostream& operator<<(std::ostream& out, const Cell& cell);

struct Cell {
	uint8_t value{};
	state_set states{};

	bool is_collapsed{false};

	Cell() {
		states.set();
	}

	/**
	 * @brief Returns the value if collapsed, otherwise returns the nth possibility
	 * @param n
	 * @return
	 */
	uint8_t operator[](int n) {
		if (is_collapsed) {
			return value;
		} else {
			int count{0};
			int i{0};

			while (i < NUM_STATES) {
				if (states[i]) {
					if (count == n) {
						return i;
					}
					count++;
				}
				i++;
			}

			return 0;
		}
	}

	/**
	 * @brief Returns the value if collapsed, otherwise returns the nth possibility
	 * @param n
	 * @return
	 */
	const uint8_t operator[](int index) const {
		if (is_collapsed) {
			return value;
		} else {
			int count{0};
			int i{0};

			while (i < NUM_STATES) {
				if (states[i]) {
					if (count == index) {
						return i;
					}
					count++;
				}
				i++;
			}

			return 0;
		}
	}

	int num_states() const {
		if (is_collapsed) {
			return 1;
		} else {
			return states.count();
		}
	}
};

std::ostream& operator<<(std::ostream& out, const uint8_t num) {
	out << static_cast<unsigned int>(num);
	return out;
}

std::ostream& operator<<(std::ostream& out, const Cell& cell) {
	out << "{ ";
	if (cell.is_collapsed) {
		out << cell.value;
	} else {
		for (int i{0}; i < NUM_STATES; i++) {
			if (cell.states[i]) {
				out << i << ", ";
			}
		}
	}
	out << " }";

	return out;
}

using Row = std::array<Cell, BOARD_SIZE>;

class Board {
private:
	std::array<Row, BOARD_SIZE> cells;

public:
	Row& operator[](const int index) {
		return cells[index];
	}

	const Row& operator[](const int index) const {
		return cells[index];
	}

	Cell& operator[](const Vec2& index) {
		return cells[index.x][index.y];
	}

	const Cell& operator[](const Vec2& index) const {
		return cells[index.x][index.y];
	}

	std::array<Row, BOARD_SIZE>::iterator begin() noexcept {
		return cells.begin();
	}

	std::array<Row, BOARD_SIZE>::const_iterator begin() const noexcept {
		return cells.begin();
	}

	std::array<Row, BOARD_SIZE>::iterator end() noexcept {
		return cells.end();
	}

	std::array<Row, BOARD_SIZE>::const_iterator end() const noexcept {
		return cells.end();
	}
};

template <typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& vec) {
	for (auto& elem : vec) {
		out << elem << ", ";
	}
	return out;
}

std::ostream& operator<<(std::ostream& out, const Vec2& vec) {
	out << "(" << vec.x << ", " << vec.y << ")";
	return out;
}

std::ostream& operator<<(std::ostream& out, const Board& board) {
	int square_size{(int)ceil(sqrt(NUM_STATES))};

	for (auto& row : board) {
		for (int i{0}; i < square_size; i++) {
			for (auto& cell : row) {
				for (int j{0}; j < square_size; j++) {
					if (cell.is_collapsed) {
						out << cell.value + 1;
					} else {
						int index{i * square_size + j};
						if (cell.states[index]) {
							out << index + 1;
						} else {
							out << " ";
						}
					}
				}
				out << " ";
			}
			out << "\n";
		}
		out << "\n";
	}

	return out;
}

std::vector<uint8_t> iter_set(const std::vector<Cell>& cell_states, int n) {
	std::vector<uint8_t> state{};

	int curr{n};

	for (const Cell& cell : cell_states) {
		if (curr != 0) {
			int c{cell.num_states()};
			int rem{curr % c};
			curr -= rem;
			curr /= c;
			state.push_back(cell[rem]);
		} else {
			state.push_back(cell[0]);
		}
	}

	return state;
}

void update_board(Board& board) {
	const int max_threads{(int)std::thread::hardware_concurrency() / 2};
	// const int max_threads{1};

	for (int i{0}; i < BOARD_SIZE; i++) {
		for (int j{0}; j < BOARD_SIZE; j++) {
			Vec2 pos{i, j};
			if (!board[pos].is_collapsed) {
				for (auto& rule : rules) {
					// Copy of the original states
					std::vector<Cell> cell_group_states{};

					for (auto& cell_pos : rule.get_cells(pos)) {
						cell_group_states.push_back(board[cell_pos]);
					}

					int num_possibilities{1};
					for (Cell& state : cell_group_states) {
						num_possibilities *= state.num_states();
					}

					int given_each{(int)ceil(num_possibilities / NUM_STATES)};

					auto thread_func = [&num_possibilities, &rule, &cell_group_states](uint8_t val) -> bool {
						for (int i{0}; i < num_possibilities; i++) {
							auto collapse_vals = iter_set(cell_group_states, i);
							collapse_vals.push_back(val);
							if(rule.is_valid(collapse_vals)) {
								//std::cout << val << " is valid in: " << collapse_vals << "\n";
								return true;
							}
						}
						return false;
					};

					state_set new_states{};
					new_states.set();

					std::vector<std::future<bool>> futures{(size_t)NUM_STATES};
					for (int k{0}; k < NUM_STATES; k++) {
						if (board[pos].states[k]) {
							futures[k] = std::async(thread_func, k);
						}
					}

					for (int k{0}; k < futures.size(); k++) {
						if (board[pos].states[k]) {
							new_states[k] = futures[k].get();
						}
					}

					// for (int k{0}; k < NUM_STATES; k++) {
					// 	new_states[k] = thread_func(k);
					// }

					board[pos].states &= new_states;
				}

				std::cout << "Cell at: " << pos << " has new states: " << board[pos] << "\n";

				if (board[pos].states.count() == 1) {
					board[pos].is_collapsed = true;
					int k{0};
					while (true) {
						if (board[pos].states[k]) {
							board[pos].value = k;
							break;
						}
						k++;
					}
				}
			}
		}
	}
}

int main() {
	Board board{};

	auto collapse_cell = [&](Vec2 pos, uint8_t value) {
		board[pos].value = value - 1;
		board[pos].is_collapsed = true;
	};

	auto update = [&]() {
		std::cout << "Updating Board\n";
		std::chrono::time_point<std::chrono::system_clock> delta{};
		auto start{std::chrono::system_clock::now()};

		update_board(board);
		delta += std::chrono::system_clock::now() - start;
		double real_time{(double)delta.time_since_epoch().count() / (std::chrono::system_clock::period::den)};

		std::cout << board << "\n";

		std::cout << real_time << "s\n";
	};

	int x_pos{};
	int y_pos{};
	uint8_t value{};

	// collapse_cell(Vec2{0, 0}, 4);
	// collapse_cell(Vec2{0, 3}, 9);
	// collapse_cell(Vec2{0, 7}, 5);
	// collapse_cell(Vec2{1, 2}, 5);
	// collapse_cell(Vec2{1, 3}, 6);
	// collapse_cell(Vec2{1, 5}, 7);
	// collapse_cell(Vec2{1, 6}, 2);
	// collapse_cell(Vec2{1, 8}, 4);
	// collapse_cell(Vec2{2, 5}, 4);
	// collapse_cell(Vec2{2, 6}, 7);
	// collapse_cell(Vec2{3, 0}, 8);
	// collapse_cell(Vec2{3, 1}, 7);
	// collapse_cell(Vec2{3, 3}, 3);
	// collapse_cell(Vec2{3, 6}, 6);
	// collapse_cell(Vec2{4, 2}, 9);
	// collapse_cell(Vec2{4, 3}, 7);
	// collapse_cell(Vec2{4, 4}, 2);
	// collapse_cell(Vec2{4, 6}, 1);
	// collapse_cell(Vec2{4, 7}, 8);
	// collapse_cell(Vec2{5, 2}, 6);
	// collapse_cell(Vec2{5, 3}, 8);
	// collapse_cell(Vec2{5, 4}, 9);
	// collapse_cell(Vec2{5, 5}, 1);
	// collapse_cell(Vec2{6, 0}, 1);
	// collapse_cell(Vec2{6, 2}, 2);
	// collapse_cell(Vec2{6, 3}, 4);
	// collapse_cell(Vec2{6, 6}, 5);
	// collapse_cell(Vec2{6, 7}, 6);
	// collapse_cell(Vec2{6, 8}, 8);
	// collapse_cell(Vec2{7, 0}, 7);
	// collapse_cell(Vec2{7, 1}, 6);
	// collapse_cell(Vec2{7, 3}, 5);
	// collapse_cell(Vec2{7, 4}, 3);
	// collapse_cell(Vec2{7, 5}, 8);
	// collapse_cell(Vec2{7, 8}, 1);
	// collapse_cell(Vec2{8, 2}, 8);
	// collapse_cell(Vec2{8, 5}, 2);
	// collapse_cell(Vec2{8, 7}, 7);

	std::string input =
		"010000504"
		"096007000"
		"000200010"
		"000000807"
		"085060002"
		"004000000"
		"030000090"
		"009030005"
		"000540060";

	int loop = 0;
	for (char c : input) {
		if (c != '0') {
			uint8_t val = c - '0';
			std::cout << val << "\n";
			collapse_cell(Vec2{loop % BOARD_SIZE, loop / BOARD_SIZE}, val);
		}
		loop++;
	}

	std::cout << board << "\n";

	while (true) {
		char cmd{};
		std::cin >> cmd;

		switch (cmd) {
			case 'U':
				update();
				break;
			case 'F':
				std::cout << "Row>";
				std::cin >> x_pos;
				std::cout << "Col>";
				std::cin >> y_pos;
				std::cout << "Num>";
				std::cin >> value;
				collapse_cell(Vec2{x_pos, y_pos}, value - '0');
		}
	}

	return 0;
}
