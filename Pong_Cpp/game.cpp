/*
This file is excluded from the build
*/
//Macros for buttons
#define is_down(b) input->buttons[b].isDown
#define pressed(b) (input->buttons[b].isDown && input->buttons[b].changed) //down and changed
#define released(b) (!input->buttons[b].isDown && input->buttons[b].changed) //not down

float player_1p, player_1dp, player_2p, player_2dp;
float arena_half_size_x=85, arena_half_size_y=45;
float player_half_size_x=2.5, player_half_size_y=12;
float ball_p_x, ball_p_y, ball_dp_x=130, ball_dp_y, ball_half_size=1; //player_dp - player derivate position

//Score system
int player_1_score, player_2_score;

internal void
simulate_player(float *p, float *dp, float ddp, float dt) {
	//friction -  change in acceleration based on the velocity
	ddp -= *dp * 10.f;

	//calculations for acceleration
	*p = *p + *dp * dt + ddp * dt * dt * .5f;
	*dp = *dp + ddp * dt;

	//collision - This is a 1D collision
	if (*p + player_half_size_y > arena_half_size_y) {//we are colliding
		*p = arena_half_size_y - player_half_size_y;
		*dp = 0; //change the velocity, invert it or set it to 0
	}
	else if (*p - player_half_size_y < -arena_half_size_y) {//For the other wall, same thing but inverted
		*p = -arena_half_size_y + player_half_size_y;
		*dp = 0; //change the velocity, invert it or set it to 0
	}

}

internal bool
aabb_vs_aabb(float p1x, float p1y, float hs1x, float hs1y,
	float p2x,float p2y, float hs2x, float hs2y) {
	return (p1x + hs1x > p2x - hs2x &&
		p1x - hs1x < p2x + hs2x &&
		p1y + hs1y > p2y - hs2y &&
		p1y + hs1y < p2y + hs2y);
}

enum Gamemode {
	GM_MENU,
	GM_GAMEPLAY,
};

Gamemode current_gamemode;
int hot_button;//to show which button we've selected
bool enemy_is_ai;
internal void
simulate_game(Input* input,float dt) {
	draw_rect(0, 0, arena_half_size_x, arena_half_size_y, 0xffaa33);
	draw_arena_borders(arena_half_size_x, arena_half_size_y, 0xff5500);

	float player_1ddp = 0.f;

	if (current_gamemode == GM_GAMEPLAY)
	{
		//GAMEPLAY MODE
		//AI Logic
		if (!enemy_is_ai)
		{
			if (is_down(BUTTON_UP))	player_1ddp += 2000;
			if (is_down(BUTTON_DOWN))	player_1ddp -= 2000;
		}
		else
		{
		//if (ball_p_y > player_1p+2f) player_1ddp += 1300;
		//if (ball_p_y < player_1p-2f) player_1ddp -= 1300;
		player_1ddp = (ball_p_y - player_1p) * 100;
		if (player_1ddp > 1300)player_1ddp = 1300;
		if (player_1ddp < -1300)player_1ddp = -1300;
		}
	float player_2ddp = 0.f;
	if (is_down(BUTTON_W))	player_2ddp += 2000;
	if (is_down(BUTTON_S))	player_2ddp -= 2000;
	//the is for stable acceleration(just assign a value to speed(units per secpnd))
	//if (is_down(BUTTON_LEFT))	player_pos_x -= speed * dt;
	//if (is_down(BUTTON_RIGHT))	player_pos_x += speed * dt;

	//simulating the player
	simulate_player(&player_1p, &player_1dp, player_1ddp, dt);
	simulate_player(&player_2p, &player_2dp, player_2ddp, dt);

	//Simulate the Ball
	{
		//ball movement
		ball_p_x += ball_dp_x * dt;
		ball_p_y += ball_dp_y * dt;


		//ball collision
		//80 player's x position (AABB)
		if (aabb_vs_aabb(ball_p_x,ball_p_y,ball_half_size,ball_half_size,80,player_1p,player_half_size_x,player_half_size_y))
		{
			//ball_dp_y = player_1dp*0.75;//changing the ball's y direction*coefficient
			//ball_dp_y = (ball_p_y - player_1p)*2; // making the ball go up and down depending on where it was hit
			ball_p_x = 80 - player_half_size_x - ball_half_size;
			ball_dp_x *= -1;
			ball_dp_y = (ball_p_y - player_1p) * 2 + player_1dp * 0.75;
		}else if (aabb_vs_aabb(ball_p_x, ball_p_y, ball_half_size, ball_half_size, -80, player_2p, player_half_size_x, player_half_size_y))
		{
			ball_p_x = -80 + player_half_size_x + ball_half_size;
			ball_dp_x *= -1;
			//ball_dp_y = player_2dp * 0.75;//changing the ball's y direction*coefficient
			ball_dp_y = (ball_p_y - player_2p) * 2 + player_2dp * 0.75;
		}

		//ball-arena collision
		//top of the arena
		if (ball_p_y + ball_half_size > arena_half_size_y) {
			ball_p_y = arena_half_size_y - ball_half_size;
			ball_dp_y *= -1;
		}
		//bottom of the arena
		else if (ball_p_y + ball_half_size < -arena_half_size_y) {
			ball_p_y = -arena_half_size_y + ball_half_size;
			ball_dp_y *= -1;
		}
		//left and right collisions
		if (ball_p_x + ball_half_size > arena_half_size_x) {
			ball_dp_x *= -1;
			ball_dp_y = 0;
			ball_p_x = 0;
			ball_p_y = 0;
			player_1_score++;
		} else if (ball_p_x - ball_half_size < -arena_half_size_x) {
			ball_dp_x *= -1;
			ball_dp_y = 0;
			ball_p_x = 0;
			ball_p_y = 0;
			player_2_score++;
		}
	}

	// drawing the number (score,x,y,size,colour)
	draw_number(player_1_score, -10, 40, 1.f,0xbbffbb);
	draw_number(player_2_score, 10, 40, 1.f, 0xbbffbb);
	//Rendering

	//drawing the ball
	draw_rect(ball_p_x, ball_p_y, ball_half_size, ball_half_size, 0xffffff);
	//drawing the players
	draw_rect(80, player_1p, player_half_size_x, player_half_size_y, 0xff0000);
	draw_rect(-80, player_2p, player_half_size_x, player_half_size_y, 0xff0000);


}
else
{
	if (pressed(BUTTON_LEFT) || pressed(BUTTON_RIGHT))
	{
		hot_button = !hot_button;
	}
	if (pressed(BUTTON_ENTER)) {
		current_gamemode = GM_GAMEPLAY;
		enemy_is_ai = hot_button ? 0 : 1;
	}
	//MENU MODE
	if (hot_button == 0){
		draw_text("SINGLE PLAYER", -80, -10, 1, 0xff0000);
		draw_text("MULTIPLAYER", 20, -10, 1, 0xaaaaaa);
	}
	else
	{
		draw_text("SINGLE PLAYER", -80, -10, 1, 0xaaaaaa);
		draw_text("MULTIPLAYER", 20, -10, 1, 0xff0000);
	}
	draw_text("PONG", -25, 40, 2, 0xffffff);
	draw_text("COMPLETELY MADE IN CPP", -50, 22, .75, 0xffffff);

}
}