#include "common.h"

#include <cctype>
#include <sstream>
#include <regex>
#include <string>
#include <iostream>
#include <cmath>

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;
const Position Position::NONE = { -1, -1 };

// –еализуйте методы:
bool Position::operator==(const Position rhs) const { return row == rhs.row && col == rhs.col; }

bool Position::operator<(const Position rhs) const {
	return
		(row < rhs.row) ||
		(row == rhs.row && col < rhs.col);

}

bool Position::IsValid() const {
	if (col < 0 || col >= MAX_COLS || row < 0 || row >= MAX_ROWS) return false;
	return true;

}

std::string Position::ToString() const {

	if (!IsValid()) return {};

	std::string result = "";
	char letter;


	if (col > 25 && col <= 701) {

		letter = col / 26;
		result += char('A' + letter - 1);
	}

	if (col > 701) {

		letter = col / 26 / 26;
		result += char('A' + letter - 1);


		letter = col / 26 % 26;
		result += char('A' + letter - 1);



	}


	letter = col % 26 + 1;
	result += char('A' + letter - 1);
	result += std::to_string(row + 1);

	return result;
}


Position Position::FromString(std::string_view str) {
	if (str.size() > MAX_POSITION_LENGTH) {
		return Position::NONE;
	}
	std::regex cell(R"/(([A-Z]{1,3})(\d{1,5}$))/"); // шаблон номера €чейки
	std::smatch cell_match;
	std::string qq(str);
	std::string cell_letters = "";
	int row = -1;
	if (std::regex_match(qq, cell_match, cell)) {
		cell_letters = cell_match[1];
		row = std::stoi(cell_match[2]) - 1;

		if (row < 0 || row >= MAX_ROWS) {
			return Position::NONE;

		}
		int col = 0;
		for (int i = cell_letters.size() - 1, j = 0; i >= 0; i--, j++) {
			col += int(cell_letters[i] - 'A' + 1) * pow(26, j);
		}
		col--;
		if (col >= MAX_COLS) {
			return Position::NONE;
		}
		return { row, col };
	}
	else {
		return Position::NONE;
	}
}


bool Size::operator==(Size rhs) const {
	return cols == rhs.cols && rows == rhs.rows;
}

bool Size::operator<(Size rhs) const {
	if (rows < rhs.rows) return true;
	if (rows == rhs.rows && cols < rhs.cols) return true;
	return false;
}