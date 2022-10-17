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
#include <span>

#define __D_SINGLE_THREAD

constexpr uint8_t BOARD_SIZE{9};
constexpr uint8_t NUM_STATES{9};

struct Vec2 {
	int x;
	int y;

	Vec2 operator/(const int mult) const {
		return Vec2{x / mult, y / mult};
	}

	Vec2 operator*(const int mult) const {
		return Vec2{x * mult, y * mult};
	}

	Vec2 operator+(const Vec2& other) const {
		return Vec2{x + other.x, y + other.y};
	}

	Vec2 operator-(const Vec2& other) const {
		return Vec2{x - other.x, y - other.y};
	}

	void operator/=(const int mult) {
		x /= mult;
		y /= mult;
	}

	void operator*=(const int mult) {
		x *= mult;
		y *= mult;
	}

	void operator+=(const Vec2& other) {
		x += other.x;
		y += other.y;
	}

	void operator-=(const Vec2& other) {
		x -= other.x;
		y -= other.y;
	}

	bool operator==(const Vec2& other) const {
		return (x == other.x && y == other.y);
	}

	bool operator!=(const Vec2& other) const {
		return !((*this) == other);
	}
};

struct Rule {
	std::function<std::vector<Vec2>(const Vec2&)> get_cells;
	std::function<bool(const std::span<uint8_t>&)> is_valid;
};

using state_set = std::bitset<NUM_STATES>;

// Checks for no duplicate cells. Used a lot so I just put it here
bool default_rule(const std::span<uint8_t>& cell_vals) {
	state_set states{};

	for (auto& val : cell_vals) {
		states.set(val);
	}

	return states.count() == cell_vals.size();
};

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

using Row = std::array<Cell, BOARD_SIZE>;

class Board {
private:
	std::array<Row, BOARD_SIZE> rows;

public:
	Row& operator[](const int index) {
		return rows[index];
	}

	const Row& operator[](const int index) const {
		return rows[index];
	}

	Cell& operator[](const Vec2& index) {
		return rows[index.x][index.y];
	}

	const Cell& operator[](const Vec2& index) const {
		return rows[index.x][index.y];
	}

	std::array<Row, BOARD_SIZE>::iterator begin() noexcept {
		return rows.begin();
	}

	std::array<Row, BOARD_SIZE>::const_iterator begin() const noexcept {
		return rows.begin();
	}

	std::array<Row, BOARD_SIZE>::iterator end() noexcept {
		return rows.end();
	}

	std::array<Row, BOARD_SIZE>::const_iterator end() const noexcept {
		return rows.end();
	}

	void reset() {
		for (auto& row : rows) {
			for (auto& cell : row) {
				if (!cell.is_collapsed) {
					cell.states.reset();
				}
			}
		}
	}
};

int row_states(const Row& row) {
	int num_states{1};

	for (auto& cell : row) {
		num_states *= cell.num_states();
	}

	return num_states;
}

std::ostream& operator<<(std::ostream& out, const uint8_t num) {
	out << static_cast<unsigned int>(num);
	return out;
}

std::ostream& operator<<(std::ostream& out, const Cell& cell) {
	out << "{ ";
	if (cell.is_collapsed) {
		out << cell.value + 1;
	} else {
		for (int i{0}; i < NUM_STATES; i++) {
			if (cell.states[i]) {
				out << i + 1 << ", ";
			}
		}
	}
	out << " }";

	return out;
}

template <typename T>
std::ostream& operator<<(std::ostream& out, const std::span<T>& vec) {
	for (auto& elem : vec) {
		out << elem << ", ";
	}
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

const std::vector<Rule> rules = {
	{ // Columns
		[](const Vec2& pos) -> std::vector<Vec2> {
			std::vector<Vec2> cells{};
			for (int i{0}; i < BOARD_SIZE; i++) {
				if (i != pos.y) cells.push_back(Vec2{pos.x, i});
			};

			return cells;
		}, default_rule
	},
	{ // Rows
		[](const Vec2& pos) -> std::vector<Vec2> {
			std::vector<Vec2> cells{};
			for (int i{0}; i < BOARD_SIZE; i++) {
				if (i != pos.x) cells.push_back(Vec2{i, pos.y});
			};

			return cells;
		}, default_rule
	},
	{ //Squares
		[](const Vec2& pos) -> std::vector<Vec2> {
			int square_size{(int)ceil(sqrt(BOARD_SIZE))};
			Vec2 square_pos = pos / square_size;
			std::vector<Vec2> cells{};
			for (int i{0}; i < square_size * square_size; i++) {
				Vec2 new_pos{square_pos * square_size};
				new_pos += Vec2{(i % square_size), (i / square_size)};
				if (new_pos != pos) cells.push_back(new_pos);
			}
			return cells;
		}, default_rule
	}
	// { // X1
	// 	[](const Vec2& pos) -> std::vector<Vec2> {
	// 		std::vector<Vec2> cells{};
	// 		if (pos.x - pos.y != 0) return cells;

	// 		for (int i{0}; i < BOARD_SIZE; i++) {
	// 			Vec2 new_pos{i, i};
	// 			if(new_pos != pos) cells.push_back(new_pos);
	// 		}

	// 		return cells;
	// 	}, default_rule
	// },
	// // X2
	// {
	// 	[](const Vec2& pos) -> std::vector<Vec2> {
	// 		std::vector<Vec2> cells{};
	// 		if (pos.x + pos.y != BOARD_SIZE - 1) return cells;

	// 		for (int i{0}; i < BOARD_SIZE; i++) {
	// 			Vec2 new_pos{BOARD_SIZE - i - 1, i};
	// 			if(new_pos != pos) cells.push_back(new_pos);
	// 		}

	// 		return cells;
	// 	}, default_rule
	// }
	// { // Dutch Miracle 1
	// 	[](const Vec2& pos) -> std::vector<Vec2> {
	// 		int rank = pos.y + pos.x - (BOARD_SIZE - 1);
	// 		int num_cells = BOARD_SIZE - abs(rank);
	// 		Vec2 start{0, 0};
	// 		if (rank < 0) {
	// 			start.x -= abs(rank);
	// 		} else {
	// 			start.y += abs(rank);
	// 		}

	// 		std::vector<Vec2> cells{};
	// 		for (int i{0}; i < num_cells; i++) {
	// 			Vec2 new_pos{BOARD_SIZE - i - 1, i};
	// 			new_pos += start;
	// 			if (new_pos != pos) cells.push_back(new_pos);
	// 		}

	// 		return cells;
	// 	}, default_rule
	// },
	// { // Dutch Miracle 2
	// 	[](const Vec2& pos) -> std::vector<Vec2> {
	// 		std::vector<Vec2> cells{};

	// 		if (pos.x > 0 && pos.y < BOARD_SIZE - 1) {
	// 			cells.push_back(Vec2{pos.x - 1, pos.y + 1});
	// 		}
	// 		if (pos.x < BOARD_SIZE - 1 && pos.y > 0) {
	// 			cells.push_back(Vec2{pos.x + 1, pos.y - 1});
	// 		}

	// 		return cells;
	// 	},
	// 	[](const std::vector<uint8_t>& cell_vals) -> bool {
	// 		if (cell_vals.size() == 1) return true;

	// 		uint8_t mid_cell = cell_vals[cell_vals.size() - 1];
	// 		bool a_diff = abs(cell_vals[0] - mid_cell) >= 4;
	// 		if (cell_vals.size() == 2) return a_diff;

	// 		bool b_diff = abs(cell_vals[1] - mid_cell) >= 4;
	// 		return a_diff && b_diff;
	// 	}
	// }
};

std::vector<uint8_t> increment_set(const std::vector<uint8_t>& curr, const std::vector<uint8_t>& bases) {
	std::vector<uint8_t> next{curr};

	int digit{(int)curr.size() - 1};
	bool carry{false};
	do {
		next[digit]++;
		if (carry) {
			next[digit + 1] = 0;
		}
		carry = next[digit] == bases[digit];
		digit --;
	} while (carry && digit >= 0);

	if (carry) {
		next[0] = 0;
	}

	// std::cout << "Original: " << curr << " +1 -> " << next << "\n";

	return next;
}

std::vector<uint8_t> iter_set(const std::span<const Cell>& cell_states, int n) {
	std::vector<uint8_t> state{};

	int curr{n};

	for (const Cell& cell : cell_states) {
		if (curr != 0) {
			if (cell.is_collapsed) {
				state.push_back(cell.value);
			} else {
				int c{cell.num_states()};
				int rem{curr % c};
				curr -= rem;
				curr /= c;
				state.push_back(cell[rem]);
			}
		} else {
			state.push_back(cell[0]);
		}
	}

	return state;
}

std::vector<std::vector<uint8_t>> iter_set(const Board& board, unsigned long long n) {
	std::vector<std::vector<uint8_t>> state{};

	unsigned long long curr{n};

	for (auto& row : board) {
		if (curr != 0) {
			int c{row_states(row)};
			int rem{static_cast<int>(curr % c)};
			curr -= rem;
			curr /= c;
			state.push_back(iter_set(row, rem));
		} else {
			state.push_back(iter_set(row, 0));
		}
	}

	return state;
}

void check_cell_collapse(Board& board, const Vec2& pos) {
	if (board[pos].states.count() == 1) {
		board[pos].is_collapsed = true;
		int i{0};
		while (!board[pos].states[i++]) {}
		board[pos].value = i - 1;
	}
}

bool update_cell(Board& board, const Vec2& pos) {
	if (board[pos].is_collapsed) return false;

	std::function<bool(uint8_t)> thread_func = [&board, &pos](uint8_t val) -> bool {
		bool possible{true};

		for (int r{0}; r < rules.size() && possible; r++) {
			auto& rule{rules[r]};
			// std::cout << "Testing new rule:\n";
			std::vector<Vec2> cell_group_locs{rule.get_cells(pos)};

			int num_cells{(int)cell_group_locs.size()};

			std::vector<Cell> cell_group_states{};

			for (auto& cell_pos : cell_group_locs) {
				cell_group_states.push_back(board[cell_pos]);
			}

			// int num_possibilities{1};
			// for (Cell& state : cell_group_states) {
			// 	num_possibilities *= state.num_states();
			// }

			// bool is_valid_for_rule{false};
			// for (int i{0}; i < num_possibilities; i++) {
			// 	auto collapse_vals = iter_set(cell_group_states, i);
			// 	collapse_vals.push_back(val);
			// 	if(rule.is_valid(collapse_vals)) {
			// 		is_valid_for_rule = true;
			// 		break;
			// 	}
			// }

			std::vector<uint8_t> bases{};
			for (Cell& state : cell_group_states) {
				bases.push_back(state.num_states());
			}

			// std::cout << "Bases Calculated (" << bases << ")\n";

			std::vector<std::vector<uint8_t>> cell_opts(num_cells);

			for (int i{0}; i < num_cells; i++) {
				if (cell_group_states[i].is_collapsed) {
					cell_opts[i].push_back(cell_group_states[i].value);
				} else {
					for (int j{0}; j < NUM_STATES; j++) {
						if (cell_group_states[i].states[j]) {
							cell_opts[i].push_back(j);
						}
					}
				}
			}

			std::vector<uint8_t> collapse_inds(num_cells);

			auto not_zero = [&collapse_inds]() -> bool {
				for (auto& val : collapse_inds) {
					if (val != 0) return true;
				}

				return false;
			};

			std::vector<uint8_t> collapse_vals(num_cells + 1);
			collapse_vals[num_cells] = val;
			bool is_valid_for_rule{false};

			do {
				for (int i{0}; i < num_cells; i++) {
					collapse_vals[i] = cell_opts[i][collapse_inds[i]];
				}
				// std::cout << collapse_vals << " from " << collapse_inds << "\r";
				if(rule.is_valid(collapse_vals)) {
					// std::cout << "\nFOUND ONE\n";
					is_valid_for_rule = true;
					break;
				}
				// std::cout << "Validity Checked\n";
				collapse_inds = increment_set(collapse_inds, bases);
			} while (not_zero());

			// std::cout << "\nRule Checked\n";
			// std::cout << "Rule valid? " << is_valid_for_rule << "\n";

			if (!is_valid_for_rule) {
				possible = false;
			}
		}

		return possible;
	};

	state_set new_states{};
#ifndef __D_SINGLE_THREAD
	std::vector<std::future<bool>> futures{(size_t)NUM_STATES};
	for (int k{0}; k < NUM_STATES; k++) {
		if (board[pos].states[k]) {
			futures[k] = std::async(thread_func, k);
		}
	}

	for (int k{0}; k < NUM_STATES; k++) {
		if (board[pos].states[k]) {
			new_states[k] = futures[k].get();
		}
	}
#else
	for (int k{0}; k < NUM_STATES; k++) {
		if (board[pos].states[k]) {
			new_states[k] = thread_func(k);
			// std::cout << "Thread func done once\n";
		}
	}
#endif
	// std::cout << "Thread func all done\n";

	bool changed{board[pos].states != new_states};
	board[pos].states = new_states;

	check_cell_collapse(board, pos);

	std::cout << "Cell at: " << pos << " has new states: " << board[pos] << "\n";

	return changed;
}

bool update_board(Board& board) {
	bool changed{false};
	for (int i{0}; i < BOARD_SIZE; i++) {
		for (int j{0}; j < BOARD_SIZE; j++) {
			Vec2 pos{i, j};
			if(update_cell(board, pos)) {
				changed = true;
			}
		}
	}

	return changed;
}

bool check_board_state(std::vector<std::vector<uint8_t>> state) {
	for (int i{0}; i < state.size(); i++) {
		for (int j{0}; j < state[i].size(); j++) {
			for (auto& rule : rules) {
				auto cells = rule.get_cells(Vec2{i, j});
				std::vector<uint8_t> cell_states{};
				for (auto& cell : cells) {
					cell_states.push_back(state[cell.x][cell.y]);
				}
				if (!rule.is_valid(cell_states)) return false;
			}
		}
	}

	return true;
}

void brute_update(Board& board) {
	Board new_board{board};
	new_board.reset();

	unsigned long long num_possibilities{1};
	for (auto& row : board) {
		num_possibilities *= row_states(row);
	}

	std::cout << num_possibilities << "\n";

	for (unsigned long long n{0}; n < num_possibilities; n++) {
		std::cout << n << "\r";
		auto states = iter_set(board, n);
		if (check_board_state(states)) {
			for (int i{0}; i < BOARD_SIZE; i++) {
				for (int j{0}; j < BOARD_SIZE; j++) {
					if (!new_board[i][j].is_collapsed) new_board[i][j].states[states[i][j]] = true;
				}
			}
		}
	}

	board = new_board;
}

int main() {
	Board board{};

	auto collapse_cell = [&board](Vec2 pos, uint8_t value) {
		board[pos].value = value - 1;
		board[pos].is_collapsed = true;
	};

	auto update = [&board]() -> bool {
		std::cout << "Updating Board\n";
		std::chrono::time_point<std::chrono::system_clock> delta{};
		auto start{std::chrono::system_clock::now()};

		bool changed = update_board(board);
		delta += std::chrono::system_clock::now() - start;
		double real_time{(double)delta.time_since_epoch().count() / (std::chrono::system_clock::period::den)};

		std::cout << board << "\n";

		std::cout << real_time << "s\n";

		return changed;
	};

	auto mega_update = [&board]() {
		std::cout << "Force Updating Board\n";
		std::chrono::time_point<std::chrono::system_clock> delta{};
		auto start{std::chrono::system_clock::now()};

		brute_update(board);
		delta += std::chrono::system_clock::now() - start;
		double real_time{(double)delta.time_since_epoch().count() / (std::chrono::system_clock::period::den)};

		std::cout << board << "\n";

		std::cout << real_time << "s\n";
	};

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

	// std::string input =
	// 	"006840000"
	// 	"000000840"
	// 	"800600000"
	// 	"007250900"
	// 	"500000080"
	// 	"049087000"
	// 	"168520070"
	// 	"405708000"
	// 	"073106000";

	// std::string input =
	// 	"000000000"
	// 	"000000000"
	// 	"000000000"
	// 	"000000000"
	// 	"000000000"
	// 	"000000000"
	// 	"000000000"
	// 	"000000000"
	// 	"102000000";

	int loop = 0;
	for (char c : input) {
		if (c != '0') {
			uint8_t val = c - '0';
			collapse_cell(Vec2{loop / BOARD_SIZE, loop % BOARD_SIZE}, val);
		}
		loop++;
	}

	std::cout << board << "\n";

	int x_pos{};
	int y_pos{};
	uint8_t value{};

	while (true) {
		char cmd{};
		std::cin >> cmd;

		switch (cmd) {
			case 'U':
				update();
				break;
			case 'W':
				while(update()) {}
				break;
			case 'B':
				mega_update();
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
