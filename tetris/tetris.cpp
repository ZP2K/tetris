#include "tetris.hpp"

// MAIN
void tetris::run()
{
	// PREPARE CONSOLE WINDOW
	this->get_console().set_title(L"TETRIS");

	// ENABLE BUFFER RENDERER
	this->get_console().toggle_buffer_render(true);

	// DRAW GAME BORDER, THIS IS WILL NOT BE TOUCHED
	// DURING GAME PLAY
	this->draw_boundary();

	// START GAME LOOP
	this->game_loop();

	// SHOW EXIT SCREEN
	this->show_exit_screen();

	// WAIT FOR INPUT
	getchar();
}

void tetris::game_loop()
{
	// SHORTEN THIS TYPE NAME
	using steady_clock_t = std::chrono::steady_clock;

	auto update_move = steady_clock_t::now();

	// SET PIECES BEFORE BEGINNING GAME LOOP
	this->get_current_piece() = this->generate_tetromino();
	this->get_next_piece() = this->generate_tetromino();

	uint64_t total_time = 0;
	uint32_t iterations = 0;

	while (true)
	{
		// TIME AT BEGINNING OF LOOP TO CAP FRAMERATE AT PRECISELY 60 FPS
		const auto start_time = steady_clock_t::now();

		// CLEAR INSIDE OF FRAME EVERY TICK
		this->clear_game_frame();

		// ONLY MOVE CONTROLLABLE TETROMINO EVERY 250 MS
		const auto time_delta = std::chrono::duration_cast<std::chrono::milliseconds>(steady_clock_t::now() - update_move);

		// RESET TIMER WHEN MOVING TETROMINO
		const auto should_move_piece = time_delta.count() > 250;
		if (should_move_piece)
			update_move = steady_clock_t::now();

		// ERASE ANY FULL LINE
		handle_full_lines();

		// DRAW GHOST TETROMINO, BEFORE MOVING TETROMINO FOR VISIBILITY
		draw_ghost_tetromino();

		// DRAW TETROMINO AT CURRENT POSITION
		this->draw_tetromino(this->get_current_piece());

		// DRAW SOLID PARTS BY ITERATING EACH ROW AND IT'S RESPECTIVE ELEMENTS
		draw_solid_parts();

		// DRAW SCORE COUNT
		draw_score();

		// DRAW NEXT TETROMINO
		draw_next_tetromino();

		// DRAW SAVED TETROMINO
		draw_saved_piece();

		// HANDLE MOVEMENT
		if (!handle_moving_tetromino(should_move_piece))
			return; // IF NEW PIECE COLLIDES, BREAK OUT OF LOOP

		// DRAW FROM BUFFER
		this->get_console().update_scene();

		// SLEEP UNTIL ~17ms HAS PASSED FOR CONSISTENT 60 FPS
		std::this_thread::sleep_until(start_time + std::chrono::milliseconds(1000 / 60));
	}
}

void tetris::draw_saved_piece()
{
	this->get_console().clear(this->get_border_width() + 2, 10, 10, 10);

	this->get_console().draw(this->get_border_width() + 3, 10, "Saved piece:", console_color::white);
	this->draw_tetromino(screen_vector(this->get_border_width() + 6, 12), this->get_saved_piece().get_piece(), this->get_saved_piece().get_piece().get_color());
}

void tetris::draw_next_tetromino()
{
	this->get_console().clear(this->get_border_width() + 2, 3, 10, 10);
	this->get_console().draw(this->get_border_width() + 3, 3, "Next up:", console_color::white);

	this->draw_tetromino(screen_vector(this->get_border_width() + 6, 5), this->get_next_piece().get_piece(), this->get_next_piece().get_piece().get_color());
}

void tetris::show_exit_screen()
{
	this->get_console().clear();
	this->get_console().draw(5, 5, "You died!", console_color::cyan);

	char buffer[50];
	sprintf_s(buffer, sizeof(buffer), "Score: %i", this->get_score());
	this->get_console().draw(5, 7, buffer, console_color::cyan);

	// RENDER FROM BUFFER
	this->get_console().update_scene();
}

// DRAW BOUNDARIES
void tetris::draw_boundary()
{
	for (int16_t i = 0; i <= this->get_border_height(); i++)
		this->get_console().fill_horizontal(0, i, this->get_piece_character(), this->get_border_width(), 3);

	this->clear_game_frame();

	this->get_console().update_scene();
}

// CLEAR INSIDE OF BORDER
void tetris::clear_game_frame()
{
	this->get_console().clear(1, 1, this->get_border_width() - 2, this->get_border_height() - 1);
}

// DRAW TETRIS PIECE, ALSO KNOWN AS TETROMINO, PART BY PART
void tetris::draw_tetromino(screen_vector position, tetromino piece, const uint8_t color_code)
{
	for (size_t part_index = 0; part_index < piece.get_size(); part_index++)
	{
		auto part = piece[part_index];
		this->get_console().draw(position.x() + part.x(), position.y() + part.y(), this->get_piece_character(), color_code);
	}
}

void tetris::draw_tetromino(tetromino_data tetromino)
{
	auto position = tetromino.get_position();

	for (size_t part_index = 0; part_index < tetromino.get_piece().get_size(); part_index++)
	{
		auto part = tetromino.get_piece()[part_index];
		this->get_console().draw(position.x() + part.x(), position.y() + part.y(), this->get_piece_character(), tetromino.get_piece().get_color());
	}
}

tetromino tetris::get_random_tetromino()
{
	// https://en.wikipedia.org/wiki/Tetris#Tetromino_colors
	const std::array<tetromino, 6> pieces =
	{
		/*
		I TETROMINO
		#
		#
		#
		#
		*/
		tetromino(console_color::green,{ { 0, 0 },{ 0, 1 },{ 0, 2 },{ 0, 3 } }),

		/*
		J TETROMINO
		###
		#
		*/
		tetromino(console_color::cyan,{ { 0, 0 },{ 1, 0 },{ 2, 0 },{ 2, 1 } }),

		/*
		L TETROMINO
		###
		#
		*/
		tetromino(console_color::red,{ { -1, 0 },{ 0, 0 },{ 1, 0 },{ -1, 1 } }),

		/*
		O TETROMINO
		##
		##
		*/
		tetromino(console_color::pink,{ { 0, 0 },{ 1, 0 },{ 0, 1 },{ 1, 1 } }),


		/*
		T TETROMINO
		###
		#
		*/
		tetromino(console_color::yellow,{ { 0, 0 },{ -1, 0 },{ 1, 0 },{ 0, 1 } }),

		/*
		Z TETROMINO
		##
		##
		*/
		tetromino(console_color::white,{ { 0, 0 },{ -1, 0 },{ 0, 1 },{ 1, 1 } }),
	};

	return pieces.at(rng::get_int<size_t>(0, 5));
}

screen_vector tetris::get_start_position()
{
	return screen_vector{ static_cast<int16_t>(this->get_border_width() / 2), 1 };
}

void tetris::draw_score()
{
	char score_buffer[25];
	sprintf_s(score_buffer, sizeof(score_buffer), "Score count: %u", this->get_score());
	this->get_console().draw(this->get_border_width() + 3, 1, score_buffer, console_color::white);
}

void tetris::draw_ghost_tetromino()
{
	auto position_copy = this->get_current_piece().get_position();

	do
	{
		++position_copy.y();
	} while (!this->does_element_collide(this->get_current_piece().get_piece(), position_copy));

	--position_copy.y();

	this->draw_tetromino(position_copy, this->get_current_piece().get_piece(), console_color::dark_grey);
}

void tetris::draw_solid_parts()
{
	for (int16_t y = 0; y < this->get_solid_pieces().get_row_count(); y++)
	{
		for (int16_t x = 0; x < this->get_solid_pieces().get_row_size(); x++)
		{
			auto solid_piece = this->get_solid_pieces().get_element(y, x);
			if (solid_piece.is_valid())
				this->get_console().draw(x, y, this->get_piece_character(), solid_piece.get_color());
		}
	}
}

void tetris::handle_full_lines()
{
	for (int16_t y = 0; y < this->get_solid_pieces().get_row_count() - 1; y++)
	{
		const auto row_size = this->get_solid_pieces().get_row_size();

		// CHECK IF ROW IS COMPLETE
		bool full = true;
		for (int16_t x = 1; x < row_size - 2; x++)
		{
			if (!this->get_solid_pieces().get_element(y, x).is_valid())
				full = false;
		}

		if (full)
		{
			// ADD ONE TO SCORE
			++this->score;

			// MOVE ALL LINES ABOVE IT DOWN, ESSENTIALLY OVERWRITING IT
			for (int16_t i = y; i > 1; i--) // GO BACKWARDS, SKIP TWO TOP ELEMENTS AS THEY ARE PART OF BORDER
			{
				this->get_solid_pieces().get_row(i) = this->get_solid_pieces().get_row(i - 1);
			}
		}
	}
}

bool tetris::handle_moving_tetromino(const bool should_move_piece)
{
	// SET TO TRUE WHEN READY TO ADD A NEW PIECE
	// ONLY DO SO WHEN MOVING PIECE STOPS
	auto add_new_piece = false;

	// MOVE TETROMINO IF PLAYER TELLS TO
	this->handle_controls(this->get_current_piece(), add_new_piece);

	// MOVE TETROMINO DOWN ONCE EVERY x MS
	if (should_move_piece)
		this->move_piece(this->get_current_piece(), add_new_piece);

	// ADD NEW PIECE WHEN CURRENT HAS BEEN LOCKED IN PLACE
	if (add_new_piece)
	{
		// LOCK MOVING PIECE IN PLACE
		this->add_solid_parts(this->get_current_piece().get_piece(), this->get_current_piece().get_position());

		// IF NEW PIECE COLLIDES, GAME OVER
		if (this->does_element_collide(this->get_next_piece().get_piece(), this->get_next_piece().get_position()))
			return false;

		// SET CURRENT PIECE TO NEXT PIECE
		this->get_current_piece() = this->get_next_piece();

		// GENERATE NEXT PIECE
		this->get_next_piece() = this->generate_tetromino();

		// RESET SWITCH BLOCK
		this->get_switched_piece() = false;
	}

	return true;
}

void tetris::add_solid_parts(tetromino& piece, screen_vector& position)
{
	for (auto part : piece.get_elements())
	{
		auto& element = this->get_solid_pieces().get_element(position.y() + part.y(), position.x() + part.x());
		element.get_color() = piece.get_color();
		element.is_valid() = true;
	}
}

void tetris::handle_controls(tetromino_data& data, bool& add_new_piece)
{
	auto position_copy = data.get_position();

	const key_handler_map_t key_handlers = {
		{
			// MOVE RIGHT
			VK_RIGHT,
			[](tetris* instance, tetromino_data& data, screen_vector vector_copy, bool& add_new_piece)
			{
				++vector_copy.x();
				if (!instance->does_element_collide(data.get_piece(), vector_copy))
					++data.get_position().x();
			}
		},
		{
			// MOVE LEFT
			VK_LEFT,
			[](tetris* instance, tetromino_data& data, screen_vector vector_copy, bool& add_new_piece)
			{
				--vector_copy.x();
				if (!instance->does_element_collide(data.get_piece(), vector_copy))
					--data.get_position().x();
			}
		},
		{
			// MOVE DOWN
			VK_DOWN,
			[](tetris* instance, tetromino_data& data, screen_vector vector_copy, bool& add_new_piece)
			{
				++vector_copy.y();
				if (!instance->does_element_collide(data.get_piece(), vector_copy))
					++data.get_position().y();
			}
		},
		{
			// ROTATE 90 DEGREES CLOCKWISE
			VK_UP,
			[](tetris* instance, tetromino_data& data, screen_vector vector_copy, bool& add_new_piece)
			{
				auto new_piece = data.get_piece().rotate();

				if (!instance->does_element_collide(new_piece, data.get_position()))
					data.get_piece() = new_piece;
			}
		},
		{
			// MOVE DOWN UNTIL COLLISION OCCURS
			VK_SPACE,
			[](tetris* instance, tetromino_data& data, screen_vector vector_copy, bool& add_new_piece)
			{

				auto collision = false;
				do
				{
					++vector_copy.y();

					collision = instance->does_element_collide(data.get_piece(), vector_copy);

					if (!collision)
						++data.get_position().y();

				} while (!collision);

				add_new_piece = true;
			}
		},
		{
			0x43,/*C KEY*/
			[](tetris* instance, tetromino_data& data, screen_vector vector_copy, bool& add_new_piece)
			{
				if (instance->get_switched_piece())
					return;

				// INITIATE SWITCH BLOCK
				instance->get_switched_piece() = true;

				// SWITCH PLACE WITH AN ALREADY SAVED PIECE?
				const auto switch_with_save = instance->get_saved_piece().valid();
				const auto saved_piece_copy = instance->get_saved_piece();

				// SAVE CURRENT PIECE
				instance->get_saved_piece() = instance->get_current_piece();
				instance->get_saved_piece().get_position() = instance->get_start_position();

				// IF SAVED PIECE WAS NOT NULL, SWITCH PLACE
				if (switch_with_save)
				{
					instance->get_current_piece() = saved_piece_copy;
				}
				else
				{
					instance->get_current_piece() = instance->get_next_piece();
					instance->get_next_piece() = instance->generate_tetromino();
				}

				// IF SAVED CHARACTER WAS ROTATED, IT MIGHT COLLIDE WITH BORDER
				// MOVE DOWN IF IT COLLIDES
				while (true)
				{
					if (!instance->does_element_collide(instance->get_current_piece().get_piece(), instance->get_current_piece().get_position()))
						break;

					++instance->get_current_piece().get_position().y();
				}
			}
		}
	};

	// ITERATE LIST OF HANDLERS AND INVOKE WHEN RESPECTIVE KEY IS PRESSED
	for (auto[vkey, handler_fn] : key_handlers)
	{
		if (console.get_key_press(vkey))
			handler_fn(this, data, position_copy, add_new_piece);
	}
}

void tetris::move_piece(tetromino_data& data, bool& add_new_piece)
{
	// MOVE TETROMINO DOWN ONCE TO CHECK FOR COLLISION
	auto copy_position = data.get_position();
	++copy_position.y();

	// IF ANY FUTURE BLOCK COLLIDES, LOCK TETROMINO IN PLACE
	if (!this->does_element_collide(data.get_piece(), copy_position))
	{
		++data.get_position().y();
	}
	else
	{
		// LOCK TETROMINO IN PLACE
		add_new_piece = true;
	}
}

bool tetris::does_element_collide(tetromino& piece, screen_vector position)
{
	auto& parts = piece.get_elements();
	for (auto part : parts)
	{
		// COLLISION! LOCK TETROMINO IN PLACE AND SPAWN A NEW TETROMINO
		if (this->collides(part, position))
			return true;
	}

	return false;
}

bool tetris::collides(screen_vector part, screen_vector position)
{
	auto absolute_position = screen_vector(position.x() + part.x(), position.y() + part.y());

	// COLLIDED WITH A SOLID PIECE?
	auto piece_collision = this->get_solid_pieces().get_element(position.y() + part.y(), position.x() + part.x()).is_valid();

	// COLLIDED WITH BORDER?
	auto border_collision =
		absolute_position.y() >= this->border_height ||		// COLLIDING WITH BOTTOM BORDER
		absolute_position.x() < 1 ||						// COLLIDING WITH LEFT BORDER
		absolute_position.x() > this->border_width - 2 ||	// COLLIDING WITH RIGHT BORDER
		absolute_position.y() < 1;							// COLLIDING WITH TOP BORDER

															// RETURN TRUE EITHER WAY, ANY COLLISION SHALL HALT MOVEMENT
	return piece_collision || border_collision;
}

tetromino_data tetris::generate_tetromino()
{
	return tetromino_data(this->get_start_position(), this->get_random_tetromino());
}


// GETTERS/SETTERS

tetromino_data& tetris::get_current_piece()
{
	return this->current_piece;
}

tetromino_data& tetris::get_next_piece()
{
	return this->next_piece;
}

tetromino_data& tetris::get_saved_piece()
{
	return this->saved_piece;
}

bool& tetris::get_switched_piece()
{
	return this->has_switched_piece;
}

console_controller& tetris::get_console()
{
	return this->console;
}

int32_t& tetris::get_border_width()
{
	return this->border_width;
}

int32_t& tetris::get_border_height()
{
	return this->border_height;
}

int16_t& tetris::get_piece_character()
{
	return this->piece_character;
}

uint32_t& tetris::get_score()
{
	return this->score;
}

array2d<solid_piece>& tetris::get_solid_pieces()
{
	return this->solid_pieces;
}


