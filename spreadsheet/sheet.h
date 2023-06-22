#pragma once

#include "cell.h"
#include "common.h"

#include <functional>

class Sheet : public SheetInterface {
public:

    Sheet() {
        
        
    }

    ~Sheet() {


    }

    //устанавливает содержимое €чейки с позицией pos
    void SetCell(Position pos, std::string text) override;

    //возращает ссылку на содержимое €чейки 
    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    //очищает €чейку с позицией pos
    void ClearCell(Position pos) override;
    
    // считает и возращает размер рабочей области дл€ печати
    Size GetPrintableSize() const override;

    //выводит содержимое таблицы в поток
    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

	


private:
    
    //€чейки таблицы
    std::vector<std::vector< std::unique_ptr<Cell>>> sheet_;

	
};