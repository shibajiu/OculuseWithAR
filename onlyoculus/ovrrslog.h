#pragma once
#include <iostream>
#include <vector>
#include <array>
#include <glm/vec3.hpp>

void WriteLog(std::vector<std::array<glm::vec3, 3>>);

void WriteLogWithSpeed(const std::vector<std::array<glm::vec3, 3>>, const std::vector<std::array<glm::vec3, 3>>);