#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {
    class Formula : public FormulaInterface {
    public:
        // Реализуйте следующие методы:
        explicit Formula(std::string expression) {
            try {
                ast_ = new FormulaAST(ParseFormulaAST(expression));
            }
            catch (...) {
                throw FormulaException("Bad Formula");
            }

        }
        Value Evaluate(const SheetInterface& sheet) const override {
            Value result;
            try {
                result = ast_->Execute(sheet);
            }
            catch (const FormulaError& fe) {
                return fe;
            }
            catch (...) {
                throw FormulaException("Can't evaluate formula");
            }
            return result;
        }
        std::string GetExpression() const override {
            std::ostringstream out;

            ast_->PrintFormula(out);
            return out.str();
        }

        std::vector<Position> GetReferencedCells() const override {

            auto temp = ast_->GetCells();
            std::vector<Position> result(temp.begin(), temp.end());
            auto it = std::unique(result.begin(), result.end());
            result.resize(std::distance(result.begin(), it));

            return result;
        }


        ~Formula() {
            delete ast_;

        }
    private:
        FormulaAST* ast_ = nullptr;
    };
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}


