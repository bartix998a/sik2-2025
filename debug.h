//
// Created by Bartek on 04.06.2025.
//

#ifndef ZAL1_DEBUG_H
#define ZAL1_DEBUG_H

#ifdef DEBUG
constexpr bool debug = true;
#endif
#ifndef DEBUG
constexpr bool debug = false;
#endif

#endif //ZAL1_DEBUG_H
