#include "c++lib.hpp"
#include "entity.hpp"
#include "system.hpp"

namespace phobos {

static entity g_cur;
std::unordered_map<entity, std::uint32_t> g_entity_mapping[static_cast<size_t>(system_id::NUM)];
static std::vector<entity> g_on_hold;

const std::vector<entity> &dead_this_tick()
{
	return g_on_hold;
}

entity spawn()
{
	return ++g_cur;
}

void despawn(entity e)
{
	assert(e);
	g_on_hold.push_back(e);
}

void update()
{
	// updates during the loop
	for (size_t i = 0; i < g_on_hold.size(); ++i) {
		const auto e = g_on_hold[i];
#define X(name) if (has_component(e, system_id::name)) system.name.remove(e);
		PHOBOS_SYSTEMS(X)
#undef X
		for (size_t type = 0; type < std::size(g_entity_mapping); ++type) {
			g_entity_mapping[type].erase(e);
		}
	}
	g_on_hold.clear();
}

} // phobos

