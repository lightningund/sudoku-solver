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

// 3x3 @9: 0.000858
// 4x4 @9: 0.017
// 5x5 @9: 0.273
// 6x6 @9: 4.31
// 7x7 @9: 56.13 -> 7 -> 3.4 -> 0.159
// 8x8 @9: ??? -> 86 -> 41 -> 1.42
// 9x9 @9: ??? -> 1102 -> ??? -> 20.628
constexpr uint8_t BOARD_SIZE{4};
constexpr uint8_t NUM_STATES{4};

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

constexpr size_t NUM_RULES{2};
const Rule rules[] = {
	// Column
	{
		[](const Vec2& pos) -> std::vector<Vec2> {
			std::vector<Vec2> cells{BOARD_SIZE};
			for (int i{0}; i < BOARD_SIZE; i++) {
				cells[i] = Vec2{pos.x, i};
			}

			return cells;
		},
		[](const std::vector<uint8_t>& cell_vals) -> bool {
			state_set states{};

			for (auto& val : cell_vals) {
				states.set(val);
			}

			return states.count() == NUM_STATES;
		}
	},
	// Row
	{
		[](const Vec2& pos) -> std::vector<Vec2> {
			std::vector<Vec2> cells{BOARD_SIZE};
			for (int i{0}; i < BOARD_SIZE; i++) {
				cells[i] = Vec2{i, pos.y};
			}

			return cells;
		},
		[](const std::vector<uint8_t>& cell_vals) -> bool {
			state_set states{};

			for (auto& val : cell_vals) {
				states.set(val);
			}

			return states.count() == NUM_STATES;
		}
	}
	// //Squares
	// {
	// 	[](const Vec2& pos) -> std::vector<Vec2> {
	// 		int square_size{(int)ceil(sqrt(BOARD_SIZE))};
	// 		Vec2 square_pos = pos / square_size;
	// 		std::vector<Vec2> cells{};
	// 		for (int i{0}; i < square_size; i++) {
	// 			for (int j{0}; j < square_size; j++) {
	// 				cells.push_back(Vec2{square_pos.x * square_size + i, square_pos.y * square_size + j});
	// 			}
	// 		}

	// 		return cells;
	// 	},
	// 	[](const std::vector<uint8_t>& cell_vals) -> bool {
	// 		state_set states{};

	// 		for (auto& val : cell_vals) {
	// 			states.set(val);
	// 		}

	// 		return states.count() == NUM_STATES;
	// 	}
	// }
};

struct Cell;

std::ostream& operator<<(std::ostream& out, const Cell& cell);

struct Cell {
	uint8_t value{};
	state_set states{};

	bool is_collapsed{false};
	std::bitset<NUM_RULES> rules_checked{};

	Cell() {
		states.set();
	}

	// uint8_t operator[](int index) {
	// 	if (is_collapsed) {
	// 		return value;
	// 	} else {
	// 		int count{0};
	// 		int i{0};

	// 		while (count < index && i < NUM_STATES) {
	// 			if (states[i]) {
	// 				if (count == index) {
	// 					std::cout << (*this) << "@ index: " << index << " is: " << i << "\n";
	// 					return i;
	// 				}
	// 				count++;
	// 			}
	// 			i++;
	// 		}

	// 		std::cout << "couldn't find anything\n";
	// 		return 0;
	// 	}
	// }

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

std::ostream& operator<<(std::ostream& out, const uint8_t num) {
	out << static_cast<unsigned int>(num);
	return out;
}

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

// O(n) runtime and memory unfortunately
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
	// const int max_threads{(int)std::thread::hardware_concurrency() / 2};
	const int max_threads{1};

	// Reset the board (basically)
	for (auto& row : board) {
		for (auto& cell : row) {
			cell.rules_checked.reset();
		}
	}

	for (int i{0}; i < BOARD_SIZE; i++) {
		for (int j{0}; j < BOARD_SIZE; j++) {
			Vec2 pos{i, j};
			if (!board[pos].is_collapsed) {
				for (int r{0}; r < NUM_RULES; r++) {
					auto& rule{rules[r]};

					if (board[pos].rules_checked[r]) continue;

					// Where the cell group cells are
					std::vector<Vec2> cell_group_locs{rule.get_cells(pos)};

					// Copy of the original states
					std::vector<Cell> cell_group_states{};

					// The new possible states under this rule
					std::vector<state_set> cell_group_new_states{};

					for (auto& cell_pos : cell_group_locs) {
						board[cell_pos].rules_checked.set(r);
						cell_group_states.push_back(board[cell_pos]);
						cell_group_new_states.push_back(state_set{});
					}

					std::cout << cell_group_states << "\n";

					int num_possibilities{1};
					for (Cell& state : cell_group_states) {
						num_possibilities *= state.num_states();
					}

					int given_each{(int)ceil(num_possibilities / max_threads)};

					auto check_possibility = [&cell_group_new_states, &cell_group_states, &rule, &r](int n) -> bool {
						auto collapse_vals = iter_set(cell_group_states, n);
						if (rule.is_valid(collapse_vals)) {
							std::cout << collapse_vals << " is valid\n";

							bool all_full{false};

							for (int i{0}; i < cell_group_new_states.size(); i++) {
								cell_group_new_states[i][collapse_vals[i]] = 1;
								// if (!cell_group_states[i].is_collapsed) {
								// 	if (cell_group_new_states[i].count() != NUM_STATES) {
								// 		all_full = false;
								// 	}
								// }
							}

							return all_full;
						}

						return false;
					};

					auto thread_func = [&check_possibility](int n_min, int delta) {
						for (int i{n_min}; i < delta + n_min; i++) {
							if(check_possibility(i)) {
								return;
							}
						}
					};

					std::vector<std::future<void>> futures{(size_t)max_threads};
					for (int k{0}; k < max_threads; k++) {
						futures[k] = std::async(thread_func, k * given_each, given_each);
					}

					for (auto& f : futures) f.get();

					for (int k{0}; k < cell_group_locs.size(); k++) {
						Cell& cell = board[cell_group_locs[k]];
						if (!cell.is_collapsed) {
							cell.states &= cell_group_new_states[k];
						}
					}
				}

				std::cout << "Cell at: " << pos << " has new states: " << board[pos] << "\n";
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

	collapse_cell(Vec2{0, 0}, 1);
	collapse_cell(Vec2{1, 1}, 2);

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
				collapse_cell(Vec2{x_pos, y_pos}, value);
		}
	}

	return 0;
}
