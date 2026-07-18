```mermaid
graph TD
    Start([Bắt đầu hàm trackStock]) --> Init[Khởi tạo: i = 0, alertCount = 0]
    Init --> LoopCheck{Vòng lặp: i < cM->count?}
    
    LoopCheck -- Đúng --> Condition{components[i].stock < 5?}
    Condition -- Đúng --> Alert[In CẢNH BÁO <br> alertCount++] --> Increment[i++]
    Condition -- Sai --> Increment
    Increment --> LoopCheck

    LoopCheck -- Sai --> FinalCheck{alertCount == 0?}
    FinalCheck -- Đúng --> PrintSafe[In: Kho hàng an toàn] --> End([Kết thúc])
    FinalCheck -- Sai --> End
```