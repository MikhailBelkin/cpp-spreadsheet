#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <map>


std::vector<Position> Cell::GetReferencedCells() const{
    return impl_.get()->GetReferencedCells();
}



bool Cell::dfs(Position v, std::map<Position, char>& cl, const std::vector<Position>& ref_cells){
    
    
    cl[v] = 1;
    for (size_t i = 0; i < ref_cells.size(); ++i) {
        auto to = ref_cells[i];
        if (cl[to] == 0) {
            if (dfs(to, cl, sheet_.GetCell(to)->GetReferencedCells())) {
                throw CircularDependencyException("Circlular refs!");
            }
        }
        else if (cl[to] == 1) {
            throw CircularDependencyException("Circlular refs!");
        }
    }
    cl[v] = 2;
    return false;
}



bool Cell::CheckCyling(Position pos){
    std::map<Position, char> cl;
    auto ref_array = sheet_.GetCell(pos)->GetReferencedCells();
    if (!ref_array.empty()) {
        for (auto ref_cell : ref_array) {
            auto sub_ref_cells = sheet_.GetCell(ref_cell)->GetReferencedCells();
            if (!sub_ref_cells.empty()) {
                if (dfs(ref_cell, cl, sub_ref_cells)) {
                    throw CircularDependencyException("Circlular refs!");
                }
            }
        }
    }
    return false;

}

void Cell::Set(std::string text)  {
    MakeBackRefInvalidated();
    
    for (auto ref_cell : ref_cells_) {
        RemoveBackRefs(ref_cell);
    }


    if (text == "") {
        impl_ = std::move(std::make_unique<EmptyImpl>(EmptyImpl()));
        
        return;
    }
    if (text[0] == '=') {
        if (text.size() != 1) {
            
            try {
                impl_ = std::make_unique<FormulaImpl>(text.substr(1, text.size() - 1));
            }
            catch (...) {
                throw FormulaException("Can't parse Formula");
            }


            return;
        }
        else {
            // если только знак равно, то это не формула а строка.
            impl_ = std::move(std::make_unique<TextImpl>(TextImpl(text)));
            return;
        }

    }

    if (text[0] != '=') {
        impl_ = std::move(std::make_unique<TextImpl>(TextImpl(text)));
        return;
    }
    return;

}

void Cell::Clear() {
    impl_ = std::move(std::make_unique<EmptyImpl>(EmptyImpl()));

}
Cell::Value Cell::GetValue() const {
    if (impl_.get()->GetType() == EMPTY) { return 0.0; }
    if (impl_.get()->GetType() == STRING) {
        std::string res = impl_.get()->Get();

        if (res[0] == '\'') {
            return res.substr(1, res.size() - 1);
        }
        

        double double_result;
        size_t index = 0;
        try {
            double_result = std::stod(res, &index);
        }
        catch (...) {
            //can't convert - return text;
            return res;
        }
        if (index!=res.size()) {
            //succefully conveted, but in fact it's string, because there ara non digit symbols
            // return string
            return res; 
        
        } 
        // converted sucseflully, return double
        
        return double_result;
    }
    if (impl_.get()->GetType() == FORMULA) {
        if (cashed_value_) return cashed_value_.value();
        cashed_value_ =  impl_.get()->Evaluate(sheet_);
        return cashed_value_.value();
    }

    return impl_.get()->Get();

}
std::string Cell::GetText() const {
    if (impl_.get()->GetType() == EMPTY) { return ""; }
    if (impl_.get()->GetType() == STRING) {
        std::string res = impl_.get()->Get();
        return res;
    }
    if (impl_.get()->GetType() == FORMULA) {
        std::string res = impl_.get()->Get();
        res = "=" + res;
        return res;
    }
    return impl_.get()->Get();
} 