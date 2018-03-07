#include "tetris.hpp"

// ENTRYPOINT
int main()
{
	auto my_console = console_controller(GetStdHandle(STD_OUTPUT_HANDLE), 400, 400);

	auto tetris_game = tetris(
		my_console, // CONSOLE HANDLE
		14,			// WIDTH
		20,			// HEIGHT
		'#');		// CHARACTER USED TO DRAW BORDER AND PIECES

	tetris_game.run();

	return 0;
}

// MAIN
void tetris::run()
{
	// PREPARE CONSOLE WINDOW
	this->console.set_title(L"TETRIS");

	// ENABLE BUFFER RENDERER
	this->console.toggle_buffer_render(true);

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
	auto update_move = std::chrono::steady_clock::now();

	this->current_piece = this->generate_tetromino();
	this->next_piece = this->generate_tetromino();

	while (true)
	{
		// TIME AT BEGINNING OF LOOP TO CAP FRAMERATE AT PRECISELY 30 FPS
		const auto start_time = steady_clock_t::now();

		// CLEAR INSIDE OF FRAME EVERY TICK
		this->clear_game_frame();

		// ONLY MOVE CONTROLLABLE TETROMINO EVERY 250 MS
		const auto time_delta = std::chrono::duration_cast<std::chrono::milliseconds>(steady_clock_t::now() - update_move);

		// RESET TIMER WHEN MOVING TETROMINO
		const auto should_move_piece = time_delta.count() > 250;
		if (should_move_piece)
			update_move = std::chrono::steady_clock::now();

		// DRAW GHOST TETROMINO, BEFORE MOVING TETROMINO FOR VISIBILITY
		draw_ghost_tetromino();

		// DRAW TETROMINO AT CURRENT POSITION
		this->draw_tetromino(this->current_piece);

		// ERASE ANY FULL LINE
		handle_full_lines();

		// DRAW SOLID PARTS BY ITERATING EACH ROW AND IT'S RESPECTIVE ELEMENTS
		draw_solid_parts();

		// DRAW SCORE COUNT
		draw_score();

		// HANDLE MOVEMENT
		if (!handle_moving_tetromino(should_move_piece))
			return; // IF NEW PIECE COLLIDES, BREAK OUT OF LOOP

		// DRAW FROM BUFFER
		this->console.update_scene();

		// SLEEP UNTIL ~16ms HAS PASSED FOR CONSISTENT 60 FPS
		std::this_thread::sleep_until(start_time + std::chrono::milliseconds(1000 / 60));
	}
}

void tetris::show_exit_screen()
{
	this->console.clear();
	this->console.draw(5, 5, "You died!", console_color::cyan);

	char buffer[50];
	sprintf_s(buffer, sizeof(buffer), "Score: %i", this->score);
	this->console.draw(5, 7, buffer, console_color::cyan);

	// RENDER FROM BUFFER
	this->console.update_scene();
}

// DRAW BOUNDARIES
void tetris::draw_boundary()
{
	const auto border_character = this->piece_character;
	for (int16_t i = 0; i <= this->border_height; i++)
		this->console.fill_horizontal(0, i, border_character, border_width, 3);

	this->clear_game_frame();

	this->console.update_scene();
}

// CLEAR INSIDE OF BORDER
void tetris::clear_game_frame()
{
	this->console.clear(1, 1, this->border_width - 2, this->border_height - 1);
}

// DRAW TETRIS PIECE, ALSO KNOWN AS TETROMINO, PART BY PART
void tetris::draw_tetromino(screen_vector position, tetromino piece, const uint8_t color_code)
{
	for (size_t part_index = 0; part_index < piece.get_size(); part_index++)
	{
		auto part = piece[part_index];
		this->console.draw(position.x() + part.x(), position.y() + part.y(), this->piece_character, color_code);
	}
}

void tetris::draw_tetromino(tetromino_data tetromino)
{
	auto position = tetromino.get_position();

	for (size_t part_index = 0; part_index < tetromino.get_piece().get_size(); part_index++)
	{
		auto part = tetromino.get_piece()[part_index];
		this->console.draw(position.x() + part.x(), position.y() + part.y(), this->piece_character, tetromino.get_piece().get_color());
	}
}

tetromino tetris::get_random_tetromino()
{
	// https://en.wikipedia.org/wiki/Tetris#Tetromino_colors
	static std::array<tetromino, 6> pieces =
	{
		/*
		I TETROMINO
		#
		#
		#
		#
		*/
		tetromino(console_color::green, { { 0, 0 },{ 0, 1 },{ 0, 2 },{ 0, 3 } }),

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
		tetromino(console_color::red,{ { 0, 0 },{ 1, 0 },{ 2, 0 },{ 0, 1 } }),

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
	return screen_vector{ static_cast<int16_t>(this->border_width / 2), 1 };
}

void tetris::draw_score()
{
	char score_buffer[25];
	sprintf_s(score_buffer, sizeof(score_buffer), "Score count: %u", this->score);
	this->console.draw(this->border_width + 3, 2, score_buffer, console_color::pink);
}

void tetris::draw_ghost_tetromino()
{
	auto position_copy = this->current_piece.get_position();

	do
	{
		++position_copy.y();
	} while (!this->does_element_collide(this->current_piece.get_piece(), position_copy));

	--position_copy.y();

	this->draw_tetromino(position_copy, this->current_piece.get_piece(), console_color::dark_grey);
}

void tetris::draw_solid_parts()
{
	for (int16_t y = 0; y < this->solid_pieces.get_row_count(); y++)
	{
		for (int16_t x = 0; x < this->solid_pieces.get_row_size(); x++)
		{
			auto solid_piece = this->solid_pieces.get_element(y, x);
			if (solid_piece.is_valid())
				this->console.draw(x, y, this->piece_character, solid_piece.get_color());
		}
	}
}

void tetris::handle_full_lines()
{
	for (int16_t y = 0; y < this->solid_pieces.get_row_count() - 1; y++)
	{
		const auto row_size = this->solid_pieces.get_row_size();
		
		// CHECK IF ROW IS COMPLETE
		bool full = true;
		for (int16_t x = 1; x < row_size - 2; x++)
		{
			if (!this->solid_pieces.get_element(y, x).is_valid())
				full = false;
		}

		if (full)
		{
			// ADD ONE TO SCORE
			++this->score;

			// MOVE ALL LINES ABOVE IT DOWN, ESSENTIALLY OVERWRITING IT
			for (int16_t i = y; i > 1; i--) // GO BACKWARDS, SKIP TWO TOP ELEMENTS AS THEY ARE PART OF BORDER
			{
				this->solid_pieces.get_row(i) = this->solid_pieces.get_row(i - 1);
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
	this->handle_controls(this->current_piece, add_new_piece);

	// MOVE TETROMINO DOWN ONCE EVERY x MS
	if (should_move_piece)
		this->move_piece(this->current_piece, add_new_piece);

	// ADD NEW PIECE WHEN CURRENT HAS BEEN LOCKED IN PLACE
	if (add_new_piece)
	{
		// LOCK MOVING PIECE IN PLACE
		this->add_solid_parts(this->current_piece.get_piece(), this->current_piece.get_position());

		// IF NEW PIECE COLLIDES, GAME OVER
		if (this->does_element_collide(this->next_piece.get_piece(), this->next_piece.get_position()))
			return false;

		// SET CURRENT PIECE TO NEXT PIECE
		this->current_piece = this->next_piece;

		// GENERATE NEXT PIECE
		this->next_piece = this->generate_tetromino();
	}

	return true;
}

void tetris::add_solid_parts(tetromino& piece, screen_vector& position)
{
	for (auto part : piece.get_elements())
	{
		auto& element = this->solid_pieces.get_element(position.y() + part.y(), position.x() + part.x());
		element.get_color() = piece.get_color();
		element.is_valid() = true;
	}
}

void tetris::handle_controls(tetromino_data& data, bool& add_new_piece)
{
	auto position_copy = data.get_position();

	const static key_handler_map_t key_handlers = {
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
				auto new_piece = data.get_piece().rotate(90.f);

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
			},
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
	auto piece_collision = this->solid_pieces.get_element(position.y() + part.y(), position.x() + part.x()).is_valid();

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
