#include "sumo.h"
#include <stdio.h>
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 400

void init()
{
    lines_init();
}

void tick(Input io, float t, float dt)
{
    persist bool init = true;
    #define GRID_W 18
    #define GRID_H 6
    #define GRID_CELLS (GRID_W*GRID_H)

    s32 grid[GRID_W*GRID_H] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1,
        1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1,
        1, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1,
        1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    };

    // #define GRID_W 4
    // #define GRID_H 4
    // #define GRID_CELLS (GRID_W*GRID_H)
    // s32 grid[GRID_W*GRID_H] = {
    //     1, 1, 1, 1,
    //     1, 0, 1, 1,
    //     1, 0, 0, 1,
    //     1, 1, 1, 1
    // };

    #define for_each_state(it) for (s32 it = 0; it < GRID_CELLS; it++)
    typedef s32 State;

    struct Belief
    {
        r32 bel[GRID_CELLS];
        r32 get(State state) { return bel[state]; }
        void set(State state, r32 p) { bel[state] = p; }

        r32 get(s32 x, s32 y) { return bel[y*GRID_W+x]; }
        void set(s32 x, s32 y, r32 p) { bel[y*GRID_W+x] = p; }
    };

    struct Transition
    {
        r32 data[GRID_CELLS*GRID_CELLS];
        r32 get(State from, State to) { return data[from*GRID_CELLS+to]; }
        void set(State from, State to, r32 p) { data[from*GRID_CELLS+to] = p; }

        r32 get(s32 from_x, s32 from_y, s32 to_x, s32 to_y)
        {
            State from = from_x + from_y*GRID_CELLS;
            State to = to_x + to_y*GRID_CELLS;
            return get(from, to);
        }
        void set(s32 from_x, s32 from_y, s32 to_x, s32 to_y, r32 p)
        {
            State from = from_x + from_y*GRID_CELLS;
            State to = to_x + to_y*GRID_CELLS;
            set(from, to, p);
        }
    };

    #define _grid(x, y) grid[(y)*GRID_W+(x)]
    #define _T(from, to) T[(from)*GRID_CELLS+(to)]
    #define __T(from_x, from_y, to_x, to_y) _T((from_x)+(from_y)*GRID_W, (to_x)+(to_y)*GRID_W)
    #define __O(state_x, state_y) O[(state_y)*GRID_W+(state_x)]

    // p(State_to = x | State_from = y)
    persist r32 T[GRID_CELLS*GRID_CELLS];

    // p(observation | State = i)
    persist r32 O[GRID_CELLS];

    #define p_observation(state) O[state]
    #define p_transition(from, to) T[(from)*GRID_CELLS+(to)]
    #define p_transition_xy(from_x, from_y, to_x, to_y) p_transition(state_from_xy(from_x, from_y), state_from_xy(to_x, to_y))
    #define state_from_xy(x, y) ((y)*GRID_W+(x))
    #define is_cell_busy(x, y) (grid[(y)*GRID_W+(x)] == 1)
    #define is_cell_open(x, y) (grid[(y)*GRID_W+(x)] == 0)

    persist Belief bel;

    if (init)
    {
        for (s32 from_y = 0; from_y < GRID_H; from_y++)
        for (s32 from_x = 0; from_x < GRID_W; from_x++)
        {
            s32 num_open = 0;
            bool sr = from_x < GRID_W-1 && is_cell_open(from_x+1, from_y);
            bool sl = from_x > 0        && is_cell_open(from_x-1, from_y);
            bool su = from_y < GRID_H-1 && is_cell_open(from_x, from_y+1);
            bool sb = from_y > 0        && is_cell_open(from_x, from_y-1);
            if (sr) num_open++;
            if (sl) num_open++;
            if (su) num_open++;
            if (sb) num_open++;

            for (s32 to_y = 0; to_y < GRID_H; to_y++)
            for (s32 to_x = 0; to_x < GRID_W; to_x++)
                p_transition_xy(from_x, from_y, to_x, to_y) = 0.0f;

            // We are equally likely to go to any open neighboring square
            // r32 p = 1.0f / num_open;
            // if (sr) p_transition_xy(from_x, from_y, from_x+1, from_y) = p;
            // if (sl) p_transition_xy(from_x, from_y, from_x-1, from_y) = p;
            // if (su) p_transition_xy(from_x, from_y, from_x, from_y+1) = p;
            // if (sb) p_transition_xy(from_x, from_y, from_x, from_y-1) = p;

            // We will always stay at a given square
            p_transition_xy(from_x, from_y, from_x, from_y) = 1.0f;

            // TODO: Based on action taken
        }

        s32 num_open = 0;
        for (s32 y = 0; y < GRID_H; y++)
        for (s32 x = 0; x < GRID_W; x++)
            if (is_cell_open(x, y)) num_open++;

        for (s32 y = 0; y < GRID_H; y++)
        for (s32 x = 0; x < GRID_W; x++)
            if (is_cell_open(x, y)) bel.set(x, y, 1.0f / num_open);

        init = false;
    }

    persist bool observe_left_busy = 0;
    persist bool observe_right_busy = 0;
    persist bool observe_up_busy = 0;
    persist bool observe_down_busy = 0;
    persist r32 error_rate = 0.2f;

    if (io_key_down(LEFT)) observe_left_busy = true;
    if (io_key_down(RIGHT)) observe_right_busy = true;
    if (io_key_down(DOWN)) observe_down_busy = true;
    if (io_key_down(UP)) observe_up_busy = true;

    if (io_key_released(RETURN))
    {
        for (s32 state = 0; state < GRID_CELLS; state++)
            O[state] = 0.0f;

        for (s32 cell_y = 0; cell_y < GRID_H; cell_y++)
        for (s32 cell_x = 0; cell_x < GRID_W; cell_x++)
        {
            r32 p = 1.0f;
            bool left_is_busy = (cell_x > 0 && is_cell_busy(cell_x-1, cell_y));
            bool right_is_busy = (cell_x < GRID_W-1 && is_cell_busy(cell_x+1, cell_y));
            bool up_is_busy = (cell_y > 0 && is_cell_busy(cell_x, cell_y-1));
            bool down_is_busy = (cell_y < GRID_H-1 && is_cell_busy(cell_x, cell_y+1));

            if (observe_left_busy == left_is_busy)
                p *= (1.0f - error_rate);
            else
                p *= error_rate;

            if (observe_right_busy == right_is_busy)
                p *= (1.0f - error_rate);
            else
                p *= error_rate;

            if (observe_up_busy == up_is_busy)
                p *= (1.0f - error_rate);
            else
                p *= error_rate;

            if (observe_down_busy == down_is_busy)
                p *= (1.0f - error_rate);
            else
                p *= error_rate;

            p_observation(state_from_xy(cell_x, cell_y)) = p;
        }

        // predict one step ahead
        Belief bel_predict = {};
        for_each_state(x_now)
        {
            r32 p = 0.0f;
            for_each_state(x_prev)
            {
                p += _T(x_prev, x_now) * bel.get(x_prev);
            }
            bel_predict.set(x_now, p);
        }

        // update with measurement
        Belief bel_update = {};
        r32 eta = 0.0f;
        for_each_state(s)
        {
            r32 p = O[s] * bel_predict.get(s);
            eta += p;
            bel_update.set(s, p);
        }

        // normalize
        for_each_state(s)
        {
            r32 p = bel_update.get(s) / eta;
            bel.set(s, p);
        }

        observe_left_busy = false;
        observe_right_busy = false;
        observe_up_busy = false;
        observe_down_busy = false;
    }

    persist bool drawing_line = false;
    persist s32 select_from = 0;
    persist s32 select_to = 0;

    if (io.mouse.left.down)
    {
        s32 x = (0.5f + 0.5f * io.mouse.ndc.x) * GRID_W;
        s32 y = (0.5f - 0.5f * io.mouse.ndc.y) * GRID_H;
        if (x < 0) x = 0;
        if (x > GRID_W-1) x = GRID_W-1;
        if (y < 0) y = 0;
        if (y > GRID_H-1) y = GRID_H-1;
        if (!drawing_line)
        {
            drawing_line = true;
            select_from = y*GRID_W+x;
        }
        else
        {
            select_to = y*GRID_W+x;
        }
    }
    else
    {
        drawing_line = false;
    }

    clearc(0.0f, 0.0f, 0.05f, 1.0f);
    lines_set_scale(1.0f, 1.0f);
    {
        r32 cw = 2.0f / GRID_W;
        r32 ch = 2.0f / GRID_H;
        for (s32 y = 0; y < GRID_H; y++)
        for (s32 x = 0; x < GRID_W; x++)
        {
            r32 x0 = -1.0f + (x + 0.1f) * cw;
            r32 y0 = +1.0f - (y + 0.9f) * ch;
            if (grid[y*GRID_W+x] == 1)
            {
                lines_set_color(0xff8833ff);
                lines_draw_rect(vec2(x0, y0), vec2(0.8f*cw, 0.8f*ch));
            }

            r32 radius = 0.5f * bel.get(x, y) * cw;
            lines_draw_circle(-1.0f + (x+0.5f)*cw, +1.0f - (y+0.5f)*ch, radius);
        }
    }
    lines_flush();

    ImGui::NewFrame();
    {
        s32 mouse_x = (0.5f + 0.5f * io.mouse.ndc.x) * GRID_W;
        s32 mouse_y = (0.5f - 0.5f * io.mouse.ndc.y) * GRID_H;
        s32 from_x = select_from % GRID_W;
        s32 from_y = select_from / GRID_W;
        s32 to_x = select_to % GRID_W;
        s32 to_y = select_to / GRID_W;
        r32 p = _T(select_from, select_to);
        ImGui::Text("p(<%d, %d> -> <%d, %d>) = %.2f",
                    from_x, from_y, to_x, to_y, p);

        if (observe_left_busy)
            ImGui::Text("left: busy");
        else
            ImGui::Text("left: open");

        if (observe_right_busy)
            ImGui::Text("right: busy");
        else
            ImGui::Text("right: open");

        if (observe_down_busy)
            ImGui::Text("down: busy");
        else
            ImGui::Text("down: open");

        if (observe_up_busy)
            ImGui::Text("up: busy");
        else
            ImGui::Text("up: open");

        for (s32 state = 0; state < GRID_CELLS; state++)
        {
            if (state_from_xy(mouse_x, mouse_y) == state)
                ImGui::Text("%d: %.2f (hover)", state, O[state]);
            else
                ImGui::Text("%d: %.2f", state, O[state]);
        }
    }
    ImGui::Render();
}

#include "sumo.cpp"
