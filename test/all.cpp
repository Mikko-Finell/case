#include "t_arbuf.hpp"
#include "t_cell.hpp"
#include "t_gc.hpp"
#include "t_timer.hpp"
#include "t_trigger.hpp"
#include "t_update.hpp"
#include "t_grid.hpp"
#include "t_neighbors.hpp"
#include "t_world.hpp"

int main() {
    t_arbuf::run();
    t_cell::run();
    t_gc::run();
    t_grid::run();
    t_neighbors::run();
    t_timer::run();
    t_trigger::run();
    t_update::run();
    t_world::run();
}
