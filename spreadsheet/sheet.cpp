#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;



void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw(InvalidPositionException("Invalid Position"));
    }

    if ((pos.row + 1) > int(sheet_.size())) {
        
        sheet_.resize(pos.row + 1);
    }

    if ((pos.col + 1) > int(sheet_[pos.row].size())) {
        sheet_[pos.row].resize(pos.col + 1);
    }

    auto temp = std::move(sheet_[pos.row][pos.col]);
    try {
        sheet_[pos.row][pos.col] = std::make_unique<Cell>(*this);
        sheet_[pos.row][pos.col].get()->Set(text);
        for (auto ref_cell : sheet_[pos.row][pos.col].get()->GetReferencedCells()) {
            auto temp = dynamic_cast<const Cell*>(GetCell(ref_cell));
            temp->AddBackRefs(pos);
        }
        sheet_[pos.row][pos.col].get()->CheckCyling(pos);
    }
    catch (CircularDependencyException& ce) {
        
        sheet_[pos.row][pos.col] = std::move(temp);
        throw;
    }
    

}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) throw InvalidPositionException("Out of range");

    if ((pos.row + 1) > int(sheet_.size())) {
        return nullptr;
    }

    if ((pos.col + 1) > int(sheet_[pos.row].size())) {
        return nullptr;
    }


    return sheet_[pos.row][pos.col].get();
}


CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid())  throw InvalidPositionException("Out of range");

    if ((pos.row + 1) > int(sheet_.size())) {
        SetCell(pos, "");
        //return sheet_[pos.row][pos.col].get();
        return nullptr; //Does not work in VS
    }

    if ((pos.col + 1) > int(sheet_[pos.row].size())) {
        SetCell(pos, "");
        //return sheet_[pos.row][pos.col].get();
        return nullptr; //Does not work in VS
    }


    return sheet_[pos.row][pos.col].get();
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) throw InvalidPositionException("Out of range");
    if ((pos.row + 1) > int(sheet_.size())) {
        return;
    }

    if ((pos.col + 1) > int(sheet_[pos.row].size())) {
        return;
    }

    if (sheet_.at(pos.row).at(pos.col)) {
        sheet_[pos.row][pos.col].reset();
    }


}

Size Sheet::GetPrintableSize() const {

    if (!sheet_.size()) {
        return { 0,0 };
    }
    int max_row = 0;;
    int max_col = 0;
    int empty_flag = true;
    for (int row = 0; row < int(sheet_.size()); row++) {
        for (int col = 0; col < int(sheet_[row].size()); col++) {
            if (sheet_[row][col]) {
                max_row = row;
                empty_flag = false;
                if (max_col < col) {
                    
                    max_col = col;
                }
            }
        }
    }
    if (empty_flag) return { 0,0 };
    return { max_row + 1, max_col + 1 };
}

void Sheet::PrintValues(std::ostream& output) const {


    struct PrintCell {
        void operator()(const double d) { out << d; }
        void operator()(const std::string& str) { out << str; }
        void operator()(const FormulaError& fe) { out << fe.ToString(); }
        std::ostream& out;
    };



    Size print_size = GetPrintableSize();
    for (int row = 0; row < print_size.rows; row++) {
        for (int col = 0; col < print_size.cols; col++) {
            if (col) {
                output << "\t";
            }

            if ((col + 1) > int(sheet_[row].size())) {
                continue; // ничего не выводим, если в этой строке дальше пусто
            }
            else {
                if (sheet_.at(row).at(col)) {
                    std::visit(PrintCell{ output }, sheet_[row][col].get()->GetValue());
                }
            }
        }
        output << "\n";
    }

}
void Sheet::PrintTexts(std::ostream& output) const {
    Size print_size = GetPrintableSize();
    for (int row = 0; row < print_size.rows; row++) {
        for (int col = 0; col < print_size.cols; col++) {
            if (col) {
                output << "\t";
            }
            if ((col + 1) > int(sheet_[row].size())) {
                continue; // ничего не выводим, если в этой строке дальше пусто
            }
            else {
                if (sheet_.at(row).at(col)) {
                    output << sheet_[row][col].get()->GetText();
                }
            }
        }
        output << "\n";
    }

}
std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}