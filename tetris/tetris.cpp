#include "stdafx.h"
#include "tetris.hpp"

const int32_t tetris::details::border_height = 20;
const int32_t tetris::details::border_width = 30;
component_vector_t tetris::details::components{};
std::vector<vector_t> tetris::details::solid_parts{};

// ENTRYPOINT
int main()
{
	tetris::run();
    return 0;
}

// MAIN
void tetris::run()
{
	console::initialise();
	console::set_title(L"TETRIS");

	// +++
	//  +
	auto t_component = component('+', { {0, 0}, {1, 0}, {2, 0}, {1, 1} });

	// #
	// #
	// #
	// #
	auto i_component = component('#', { {0, 0}, {0, -1}, {0, -2}, {0, -3} });

	// DRAW GAME BORDER
	tetris::details::draw_boundary();

	auto tick_count = GetTickCount64();

	// ENTITY/COMPONENT LIST IS FORMATTED AS: POSITION, COMPONENT, IS_MOVING
	tetris::details::components.emplace_back(vector_t(tetris::details::border_width / 2, 10), true, i_component);



	// ENGINE LOOP
	while (true)
	{
		// CLEAR INSIDE OF FRAME EVERY TICK
		tetris::details::clear_game_frame();

		// ONLY MOVE CONTROLLABLE COMPONENT EVERY 500 ms
		auto move_component = GetTickCount64() - tick_count > 250;

		// RESET TIMER WHEN MOVING COMPONENT
		if (move_component)
			tick_count = GetTickCount64();

		auto add_new_component = false;
		for (auto& [position, is_moving, comp] : tetris::details::components)
		{
			auto& [position_x, position_y] = position;

			tetris::details::draw_component(position_x, position_y, comp);

			// CHECK COLLISION IF COMPONENT IS MOVING
			if (move_component && is_moving)
			{
				// IF ANY FUTURE BLOCK COLLIDES, LOCK COMPONENT IN PLACE
				auto& parts = comp.get_elements();
				for (auto part : parts)
				{
					// COLLIDED WITH ANOTHER SOLID COMPONENT?
					auto& solid_parts = tetris::details::solid_parts;
					auto component_collision = std::find(solid_parts.begin(), solid_parts.end(), part) != solid_parts.end(); 
					
					// COLLIDED WITH BORDER?
					auto new_y = part.second/*delta*/ + position_y/*base*/;

					auto border_collision = new_y >= tetris::details::border_height - 1;

					// COLLISION!
					// LOCK COMPONENT IN PLACE AND SPAWN A NEW COMPONENT
					if (component_collision || border_collision)
					{
						is_moving = false;

						// ADD CURRENT COMPONENT TO SOLID PARTS
						solid_parts.insert(solid_parts.end(), parts.begin(), parts.end());

						add_new_component = true;
					}
				}

				// MOVE COMPONENT
				if (is_moving)
					++position_y;

			}
		}

		// ENTITY/COMPONENT LIST IS FORMATTED AS: POSITION, COMPONENT, IS_MOVING
		if (add_new_component)
			tetris::details::components.emplace_back(vector_t(tetris::details::border_width / 2, 10), true, i_component);

		Sleep(1);
	}
}

// DRAW BOUNDARIES
void tetris::details::draw_boundary()
{
	const auto border_character = L'█';
	for (size_t i = 0; i <= border_height; i++)
		console::fill_horizontal(0, i, border_character, border_width);

	tetris::details::clear_game_frame();
}

// CLEAR INSIDE OF BORDER
void tetris::details::clear_game_frame()
{
	console::clear(1, 1, tetris::details::border_width - 2, tetris::details::border_height - 1);
}

// DRAW TETRIS COMPONENT PART BY PART
void tetris::details::draw_component(const int16_t x, const int16_t y, component comp)
{
	for (size_t part_index = 0; part_index < comp.get_size(); part_index++)
	{
		auto[part_x, part_y] = comp[part_index];
		console::draw(x + part_x, y + part_y, comp.get_character());
	}
}
