#pragma once
#include <string>
#include <optional>
#include <algorithm>
#include "common.h"
#include "formula.h"
#include <map>






using namespace std::string_literals;


class Cell : public CellInterface {
public:

    enum CellType
    {
        EMPTY,
        STRING, 
        FORMULA
    };

    using Value = std::variant<std::string, double, FormulaError>;

    


    Cell(const SheetInterface& sheet):sheet_(sheet) {}
    ~Cell() {}

    //устанавливает содержимое €чейки
    void Set(std::string text);
    
    //очищает содержимое €чейки
    void Clear();

    //возвращает сожержимое €чейки
    Value GetValue() const override;
    std::string GetText() const override;

    // возвращает список €чеек, на которые ссылаетс€ текуща€
    std::vector<Position> GetReferencedCells() const override;


    
    //инвалидирует кэшированное значение
    void Invalidate() const {
        cashed_value_.reset();
    }


   //инвалидирует кэш у всех зависимых €чеек
    void MakeBackRefInvalidated() {
        Invalidate();
        for (auto ref_cells : back_ref_cells_) {
            auto temp = dynamic_cast<const Cell*>(sheet_.GetCell(ref_cells));
            temp->Invalidate();
        }
        
    }

    //добавл€ет в список зависимых €чеек €чейку с позицией pos
    void AddBackRefs(Position pos) const {
        back_ref_cells_.push_back(pos);
    }


    //удал€ет ссылки на €чейку с позицией pos из списка зависимых
    void RemoveBackRefs(Position pos) const {
        back_ref_cells_.erase(std::find(back_ref_cells_.begin(), back_ref_cells_.end(), pos));

    }


    //проверка на цикличность ссылок
    bool CheckCyling(Position pos);






private:

    //проверка графа на цикличность
    bool dfs(Position v, std::map<Position, char>& cl, const std::vector<Position>& ref_cells);
    


private:


    class Impl {

    public:
        Impl() {}
        virtual std::string Get() const = 0;
        virtual void Set(std::string str) = 0;
        CellType GetType() { return type_; }
        virtual Value Evaluate(const SheetInterface& sheet) = 0;
        virtual std::vector<Position> GetReferencedCells() const = 0;
        virtual ~Impl() {};
    protected:
        CellType type_=EMPTY;
    };

    
    class EmptyImpl:public Impl {
    public:
        EmptyImpl(){
            type_ = EMPTY;
        
        };
        virtual std::string Get() const override { return ""s; }
        virtual void Set(std::string) override{
            return;
        }
        virtual Value Evaluate(const SheetInterface& sheet) override{ return ""; }
        std::vector<Position> GetReferencedCells() const override { return {}; }
        virtual ~EmptyImpl() override {};
    };

    class TextImpl:public Impl {
    public:
        TextImpl(std::string str):str_(str) {
            type_ = STRING;
        }
        virtual std::string Get() const override { return str_; }
        virtual void Set(std::string str) override{ str_ = str; }
        virtual Value Evaluate(const SheetInterface& sheet) override{ return str_; }
        std::vector<Position> GetReferencedCells() const override { return {}; }
        virtual ~TextImpl() override { return; };
    private:
        std::string str_;
    };

    class FormulaImpl:public Impl {
    public:

        struct FormulaInterFaceValue{

            void operator()(const double value) { val = value; }
            void operator()(const FormulaError& fe) { val =  fe; }

            Value val;

        };

        FormulaImpl(std::string str) :f_(std::move(ParseFormula(str))) {
            type_ = FORMULA;
            ref_cells_ = f_.get()->GetReferencedCells();
        }
        virtual std::string Get() const override { return f_.get()->GetExpression(); }

        Value Evaluate(const SheetInterface& sheet) override{
            FormulaInterFaceValue fiv;
            std::visit(fiv, f_.get()->Evaluate(sheet));
            return fiv.val;
        }

        std::vector<Position> GetReferencedCells() const {
            return ref_cells_;
        }

        virtual void Set(std::string str) override{
            std::unique_ptr<FormulaInterface> tmp;
            try {
                tmp = std::move(ParseFormula(str));
            }
            catch (const FormulaError& ) {
                
            }
            catch (...) {
                throw FormulaException("Cant evaluate formula");
            }
            f_.swap(tmp);
            ref_cells_ = f_.get()->GetReferencedCells();

        }
        virtual ~FormulaImpl() override {
        };
    private:
        std::unique_ptr<FormulaInterface> f_;
        std::vector<Position> ref_cells_;
    };



    std::unique_ptr<Impl> impl_;
    std::vector<Position> ref_cells_;
    mutable std::vector<Position> back_ref_cells_;
    mutable std::optional<Value> cashed_value_;
    const SheetInterface& sheet_;

};