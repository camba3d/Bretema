#pragma once

#include <cstdint>
#include <cmath>
#include <limits>

#include <functional>
#include <algorithm>

#include <fstream>
#include <memory>

#include <span>
#include <map>
#include <set>
#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <typeinfo>

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

#define sPtr std::shared_ptr
#define sNew std::make_shared

#define uPtr std::unique_ptr
#define uNew std::make_unique

template<typename K, typename V>
using umap = std::unordered_map<K, V>;

template<typename T>
using uset = std::unordered_set<T>;