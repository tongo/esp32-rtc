#pragma once
#include <cstdint>

// Questa struct è il "contratto" tra chi legge i sensori e chi scrive su SD
struct DataPackage {
    uint32_t timestamp; // ms dall'avvio
    float speed;
    float temp;
    int rpm;
};