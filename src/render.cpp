#include "c++lib.hpp"
#include "system.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>


namespace phobos {

static constexpr size_t MAX_TRAILS = 16zu;

struct vavb {
	GLuint va;
	GLuint vb;
};

static vavb describe_layout_f2f2(void *vdata, size_t vsize, GLuint *idata, size_t isize, GLenum usage)
{
	GLuint va;
	glGenVertexArrays(1, &va);
	glBindVertexArray(va);
	GLuint vb;
	glGenBuffers(1, &vb);
	glBindBuffer(GL_ARRAY_BUFFER, vb);
	glBufferData(GL_ARRAY_BUFFER, vsize, vdata, usage);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(0*sizeof(float)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
	glEnableVertexAttribArray(1);
	GLuint ib;
	glGenBuffers(1, &ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, isize, idata, GL_STATIC_DRAW);
	glBindVertexArray(0);
	return {va, vb};
}

static vavb describe_layout_f2_nexto_f2(const void *vdata, size_t vsize, const void *uvdata, size_t uvsize, const GLuint *idata, size_t isize, GLenum usage)
{
	GLuint va;
	glGenVertexArrays(1, &va);
	glBindVertexArray(va);
	GLuint vb;
	glGenBuffers(1, &vb);
	glBindBuffer(GL_ARRAY_BUFFER, vb);
	glBufferData(GL_ARRAY_BUFFER, vsize, vdata, usage);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);
	glEnableVertexAttribArray(0);
	glGenBuffers(1, &vb);
	glBindBuffer(GL_ARRAY_BUFFER, vb);
	glBufferData(GL_ARRAY_BUFFER, uvsize, uvdata, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);
	glEnableVertexAttribArray(1);
	GLuint ib;
	glGenBuffers(1, &ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, isize, idata, GL_STATIC_DRAW);
	glBindVertexArray(0);
	return {va, vb};
}

struct trail_buffers {
	GLuint va;
	GLuint wpos;
	GLuint ts;
};

static trail_buffers describe_layout_trail(size_t segment_count)
{
	std::vector<glm::vec2> uv;
	std::vector<GLuint> tri;
	uv.emplace_back(0.0f, 0.0f);
	uv.emplace_back(0.0f, 1.0f);
	for (size_t i = 1; i < segment_count; ++i) {
		const auto prog = static_cast<float>(i) / static_cast<float>(segment_count-1);
		uv.emplace_back(prog, 0.0f);
		uv.emplace_back(prog, 1.0f);
		const auto end = uv.size();
		// -1 -3
		// -2 -4
		// provoking vertex is the last of a triangle
		// and we want to take the "newest" one so
		// that old trails do not show
		tri.emplace_back(end-2);
		tri.emplace_back(end-4);
		tri.emplace_back(end-3);
		tri.emplace_back(end-1);
		tri.emplace_back(end-2);
		tri.emplace_back(end-3);
	}

	trail_buffers buf;
	glGenVertexArrays(1, &buf.va);
	glBindVertexArray(buf.va);
	GLuint vb;
	glGenBuffers(1, &vb);
	glBindBuffer(GL_ARRAY_BUFFER, vb);
	glBufferData(GL_ARRAY_BUFFER, uv.size() * sizeof uv[0], uv.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &buf.wpos);
	glBindBuffer(GL_ARRAY_BUFFER, buf.wpos);
	glBufferData(GL_ARRAY_BUFFER, uv.size() * 2*sizeof(float), nullptr, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
	glGenBuffers(1, &buf.ts);
	glBindBuffer(GL_ARRAY_BUFFER, buf.ts);
	glBufferData(GL_ARRAY_BUFFER, uv.size() * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(2);
	GLuint ib;
	glGenBuffers(1, &ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, tri.size() * sizeof tri[0], tri.data(), GL_STATIC_DRAW);
	glBindVertexArray(0);
	return buf;
}

full_wall_mesh load_wall_mesh()
{
	static constexpr float scale = 4.0f;
	static const glm::vec2 vert[] = {
		{ scale * -0.9f, scale * -0.9f, },
		{ scale * +0.9f, scale * -0.9f, },
		{ scale * +0.9f, scale * +0.9f, },
		{ scale * -0.1f, scale * +0.9f, },
		{ scale * +0.1f, scale * +0.3f, },
		{ scale * -0.9f, scale *  0.0f, },
	};
	static const glm::vec2 uv[] = {
		{ -0.9f, -0.9f, },
		{ +0.9f, -0.9f, },
		{ +0.9f, +0.9f, },
		{ -0.1f, +0.9f, },
		{ +0.1f, +0.3f, },
		{ -0.9f,  0.0f, },
	};
	static const GLuint indx[] = {
		0, 1, 5,
		1, 5, 4,
		4, 1, 2,
		2, 4, 3,
	};
	return full_wall_mesh{
		{{std::cbegin(vert), std::cend(vert)}},
		{ std::cbegin(uv  ), std::cend(uv  ) },
		{ std::cbegin(indx), std::cend(indx) },
	};
}

int render::init()
{
	using namespace std::literals;
	camera_pos.x = 0.0f;
	camera_pos.y = 0.0f;
	size_t ok = 0;
	shader_pipeline shader{
		"#version 410 core\n"
		"layout(location=0) in vec2 attr_pos;\n"
		"layout(location=1) in vec2 attr_uv;\n"
		"out vec2 vert_uv;\n"
		"uniform mat3 unif_view;\n"
		"uniform mat3x2 unif_model;\n"
		"uniform float world_zoom;\n"
		"void main() {\n"
			"vert_uv = attr_uv;\n"
			"mat3 model = mat3(vec3(unif_model[0], 0.0), vec3(unif_model[1], 0.0), vec3(unif_model[2], 1.0));\n"
			"vec3 pos = world_zoom * unif_view * model * vec3(attr_pos, 1.0);\n"
			"gl_Position = vec4(pos.xy, 0.0, 1.0);\n"
		"}\n\0"sv,

		"#version 410 core\n"
		"in vec2 vert_uv;\n"
		"out vec4 frag_color;\n"
		"uniform sampler2D unif_color;\n"
		"uniform float unif_red_shift;\n"
		"void main() {\n"
			"vec4 color = texture(unif_color, vert_uv);\n"
			"color.r += unif_red_shift;\n"
			"color.g -= unif_red_shift;\n"
			"color.b -= unif_red_shift;\n"
			"frag_color = color;\n"
		"}\n\0"sv,
	};

	shader_pipeline trail_shader{
		"#version 410 core\n"
		"layout(location=0) in vec2 attr_wpos;\n"
		"layout(location=1) in vec2 attr_uv;\n"
		"layout(location=2) in float attr_timestamp;\n"
		"out vec2 vert_uv;\n"
		"flat out float vert_scale;\n"
		"uniform float world_zoom;\n"
		"uniform float now;\n"
		"uniform float max_dt;\n"
		"uniform mat3 unif_view;\n"
		"void main() {\n"
		"	vert_uv = attr_uv;\n"
		"	vert_scale = 1.0 - min(max_dt, now - attr_timestamp) / max_dt;\n"
		"	vec3 pos = world_zoom * unif_view * vec3(attr_wpos, 1.0);\n"
		"	gl_Position = vec4(pos.xy, 0.0, 1.0);\n"
		"}\n\0"sv,

		"#version 410 core\n"
		"in vec2 vert_uv;\n"
		"flat in float vert_scale;\n"
		"out vec4 frag_color;\n"
		"uniform sampler2D unif_color;\n"
		"void main() {\n"
		"	vec4 color = texture(unif_color, vert_uv);\n"
		"	frag_color = vert_scale * color;\n"
		"}\n\0"sv
	};

	shader_pipeline hp_shader{
		"#version 410 core\n"
		"layout(location=0) in vec2 attr_pos;\n"
		"layout(location=1) in vec2 attr_uv;\n"
		"out vec2 vert_uv;\n"
		"out float vert_x_prog;\n"
		"uniform mat3 unif_view;\n"
		"uniform mat3 unif_model;\n"
		"void main() {\n"
			"vert_uv = attr_uv;\n"
			"vert_x_prog = attr_pos.x + 0.5;\n"
			"vec3 pos = unif_view * unif_model * vec3(attr_pos, 1.0);\n"
			"gl_Position = vec4(pos.xy, 0.0, 1.0);\n"
		"}\n\0"sv,

		"#version 410 core\n"
		"in vec2 vert_uv;\n"
		"in float vert_x_prog;\n"
		"out vec4 frag_color;\n"
		"uniform sampler2D unif_color;\n"
		"uniform float unif_fullness;\n"
		"void main() {\n"
		"	vec2 uv = (unif_fullness > vert_x_prog)? vert_uv: 0.5 + vert_uv;\n"
		"	vec4 color = texture(unif_color, uv);\n"
		"	frag_color = color;\n"
		"}\n\0"
	};
	if (!shader.ok()) goto fail;
	if (!trail_shader.ok()) goto fail;
	if (!hp_shader.ok()) goto fail;

	{
		image img{"res/basic.png\0"sv};
		if (!img.ok()) goto fail;
		texture tex{img, shader, "unif_color\0"sv};
		img.fini();

		float vdata[] = {
			-0.5f, -0.5f, 0.0f, 0.0f,
			+0.5f, -0.5f, 1.0f, 0.0f,
			+0.5f, +0.5f, 1.0f, 1.0f,
			-0.5f, +0.5f, 0.0f, 1.0f,
		};
		GLuint idata[] = {
			0, 1, 2,
			2, 3, 0,
		};
		GLuint va = describe_layout_f2f2(vdata, sizeof vdata, idata, sizeof idata, GL_STATIC_DRAW).va;
		ctx[player] = per_draw{ va, shader, tex, std::size(idata) };
		++ok;
	}

	{
		image img{"res/enemy.png\0"sv};
		if (!img.ok()) goto fail;
		texture tex{img, shader, "unif_color\0"sv};
		img.fini();

		float vdata[] = {
			-0.5f, -0.5f, 0.0f, 0.0f,
			+0.5f, -0.5f, 1.0f, 0.0f,
			+0.5f, +0.5f, 1.0f, 1.0f,
			-0.5f, +0.5f, 0.0f, 1.0f,
		};
		GLuint idata[] = {
			0, 1, 2,
			2, 3, 0,
		};
		GLuint va = describe_layout_f2f2(vdata, sizeof vdata, idata, sizeof idata, GL_STATIC_DRAW).va;
		ctx[enemy] = per_draw{ va, shader, tex, std::size(idata) };
		++ok;
	}

	{
		image img{"res/aggro.png\0"sv};
		if (!img.ok()) goto fail;
		texture tex{img, shader, "unif_color\0"sv};
		img.fini();
		float vdata[] = {
			-0.5f, -0.5f, 0.0f, 0.0f,
			+0.5f, -0.5f, 1.0f, 0.0f,
			-0.5f, +0.5f, 0.0f, 1.0f,
			+0.5f, +0.5f, 1.0f, 1.0f,
		};
		GLuint idata[] = {
			0, 1, 2,
			2, 3, 0,
		};
		GLuint va = describe_layout_f2f2(vdata, sizeof vdata, idata, sizeof idata, GL_STATIC_DRAW).va;
		ctx[aggro] = per_draw{ va, shader, tex, std::size(idata) };
		++ok;
	}

	{
		unsigned char fill[4] = { 0xf2, 0xde, 0xe3, 0xff };
		image img;
		img.base = fill;
		img.width = 1;
		img.height = 1;
		img.channels = 4;
		texture tex{img, shader, "unif_color\0"sv};

		float vdata[] = {
			0.0f, 0.0f, 0.0f, 0.0f,
			// filled in at draw time
			0.0f, 0.0f, 1.0f, 1.0f,
			0.0f, 1.0f, 0.0f, 1.0f,
		};
		GLuint idata[] = {
			0, 1, 2,
		};
		const auto bos = describe_layout_f2f2(vdata, sizeof vdata, idata, sizeof idata, GL_DYNAMIC_DRAW);
		ctx[attack_cone] = per_draw{ bos.va, shader, tex, std::size(idata) };
		attack_cone_mesh_vb = bos.vb;
		++ok;
	}

	{
		image img{"res/slash.png\0"sv};
		if (!img.ok()) goto fail;
		texture tex{img, trail_shader, "unif_color\0"sv};
		img.fini();
		auto buf = describe_layout_trail(TRAIL_MAX_SEGMENTS);
		ctx[trail] = per_draw{ buf.va, trail_shader, tex, (TRAIL_MAX_SEGMENTS-1)*6 };
		trails.wpos = buf.wpos;
		trails.ts = buf.ts;
		++ok;
	}

	{
		image img{"res/hp_bar.png\0"sv};
		if (!img.ok()) goto fail;
		texture tex{img, hp_shader, "unif_color\0"sv};
		img.fini();

		float vdata[] = {
			-0.5f, -0.5f, 0.0f, 0.0f,
			+0.5f, -0.5f, 0.5f, 0.0f,
			+0.5f, +0.5f, 0.5f, 0.5f,
			-0.5f, +0.5f, 0.0f, 0.5f,
		};
		GLuint idata[] = {
			0, 1, 2,
			2, 3, 0,
		};
		GLuint va = describe_layout_f2f2(vdata, sizeof vdata, idata, sizeof idata, GL_STATIC_DRAW).va;
		ctx[hp_bar] = per_draw{ va, hp_shader, tex, std::size(idata) };
		++ok;
	}

	// wall_mesh filled later
	if (ok == NUM-1)
		return 0;
fail:
	std::print("[GFX] Failed to load graphics assets\n");
	return 1;
}

void render::fini()
{
	for (size_t obj = 0; obj < std::size(ctx); ++obj) {
		glDeleteVertexArrays(1, &ctx[obj].va);
		ctx[obj].tex.fini();
		ctx[obj].shader.fini();
	}
}

void render::drawable(entity e, object type)
{
	assert(type != trail);
	assert(type != wall_mesh);
	const std::uint32_t type_idx = type;
	drawing_[type].emplace_back(e);
	const std::uint32_t idx = type_idx | drawing_[type].size()-1 << type_shift;
	add_component(e, system_id::render);
	reindex(e, system_id::render, idx);
}

void render::trailable(entity t, entity ref)
{
	// timestamp being 0.0 means effectively nothing is drawn
	drawing_[trail].emplace_back(t);
	auto &at = trails.trailing_.emplace_back(trail_t{});
	at.ref = ref;
	add_component(t, system_id::render);
	const std::uint32_t idx = trail | drawing_[trail].size()-1 << type_shift;
	reindex(t, system_id::render, idx);
}

void render::wall(entity e, full_wall_mesh const &mesh)
{
	using namespace std::literals;
	shader_pipeline shader{
		"#version 410 core\n"
		"layout(location=0) in vec2 attr_pos;\n"
		"layout(location=1) in vec2 attr_uv;\n"
		"out vec2 vert_uv;\n"
		"uniform mat3 unif_view;\n"
		"uniform mat3x2 unif_model;\n"
		"uniform float world_zoom;\n"
		"void main() {\n"
			"vert_uv = attr_uv;\n"
			"mat3 model = mat3(vec3(unif_model[0], 0.0), vec3(unif_model[1], 0.0), vec3(unif_model[2], 1.0));\n"
			"vec3 pos = world_zoom * unif_view * model * vec3(attr_pos, 1.0);\n"
			"gl_Position = vec4(pos.xy, 0.0, 1.0);\n"
		"}\n\0"sv,

		"#version 410 core\n"
		"in vec2 vert_uv;\n"
		"out vec4 frag_color;\n"
		"uniform sampler2D unif_color;\n"
		"uniform float unif_red_shift;\n"
		"void main() {\n"
			"vec4 color = texture(unif_color, vert_uv);\n"
			"color.r += unif_red_shift;\n"
			"color.g -= unif_red_shift;\n"
			"color.b -= unif_red_shift;\n"
			"frag_color = color;\n"
		"}\n\0"sv,
	};
	unsigned char fill[4] = { 0xd0, 0x90, 0x10, 0xff };
	image img;
	img.width = 1;
	img.height = 1;
	img.base = fill;
	img.channels = 4;
	texture tex{img, shader, "unif_color\0"sv};
	GLuint va = describe_layout_f2_nexto_f2(
		mesh.pos.data(), mesh.pos.size() * sizeof(mesh.pos[0]),
		mesh.uv .data(), mesh.uv .size() * sizeof(mesh.uv [0]),
		mesh.indices.data(), mesh.indices.size() * sizeof(mesh.indices[0]),
		GL_STATIC_DRAW
	).va;
	ctx[wall_mesh] = per_draw{ va, shader, tex, mesh.indices.size() };
	add_component(e, system_id::render);
	drawing_[wall_mesh].emplace_back(e);
	const std::uint32_t idx = trail | drawing_[wall_mesh].size()-1 << type_shift;
	reindex(e, system_id::render, idx);
}

void render::update(float now, float dt)
{
	for (size_t i = 0; i < trails.trailing_.size(); ++i) {
		auto &data = trails.trailing_[i];
		auto ref = system.tfms.world(data.ref);
		data.buf[data.insert].base = ref.pos();
		data.buf[data.insert].offs = ref.pos() + ref.y();
		data.timestamp[data.insert] = glm::vec2{now, now};
		data.insert = (data.insert+1) % TRAIL_MAX_SEGMENTS;
	}
	const auto camera_dim_i = system.input.win.dims();
	const glm::vec2 camera_dim{static_cast<float>(camera_dim_i.x), static_cast<float>(camera_dim_i.y)};
	const auto aspect_ratio = camera_dim.x / camera_dim.y;
	// translate THEN scale, so the scale is also applied to the offsets
	const glm::mat3 view{
		{ 1.0f        , 0.0f                       , 0.0f },
		{ 0.0f        , aspect_ratio               , 0.0f },
		{ camera_pos.x, camera_pos.y * aspect_ratio, 1.0f },
	};
	const float world_zoom = system.input.win.get_world_zoom();
	for (size_t obj = 0; obj < NUM; ++obj) {
		const auto &this_draw = ctx[obj];
		this_draw.shader.bind();
		this_draw.tex.bind(0);
		glBindVertexArray(this_draw.va);
		glUniformMatrix3fv(glGetUniformLocation(this_draw.shader.id, "unif_view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniform1f(glGetUniformLocation(this_draw.shader.id, "world_zoom"), world_zoom);
		if (obj != trail) for (const auto e : drawing_[obj]) {
			auto this_entity = system.tfms.world(e);
			glUniformMatrix3x2fv(glGetUniformLocation(this_draw.shader.id, "unif_model"),
					1, GL_FALSE, &this_entity[0][0]);
			const auto begin = std::cbegin(system.phys.colliding);
			const auto end   = std::cend  (system.phys.colliding);
			const auto red_shift = std::find_if(begin, end, [=] (auto elem) { return elem.main == e | elem.other == e; }) != end? 0.3f: 0.0f;
			// TODO: this should be a FSM hurt state to avoid searching every collision every time as well as handle the cooldown
			// glUniform1f(glGetUniformLocation(this_draw.shader.id, "unif_red_shift"), red_shift);
			if (obj == attack_cone) {
				const float scale = 1e-2 / dt;
				// (1-A)v+Au to spread the cone over a larger area
				const glm::vec2 slash_tail{ scale, 1.0f - scale };
				glBindBuffer(GL_ARRAY_BUFFER, attack_cone_mesh_vb);
				glBufferSubData(GL_ARRAY_BUFFER, sizeof(float[4]), sizeof slash_tail, &slash_tail);
			} else if (obj == hp_bar) {
				const auto parent = system.tfms.referential(e)->parent;
				const auto hp = system.hp.living_[index(parent, system_id::hp)];
				glUniform1f(glGetUniformLocation(this_draw.shader.id, "unif_fullness"), hp.current / hp.max);
			}
			glDrawElements(GL_TRIANGLES, this_draw.tricount, GL_UNSIGNED_INT, nullptr);
		} else {
			glUniform1f(glGetUniformLocation(this_draw.shader.id, "now"), now);
			glUniform1f(glGetUniformLocation(this_draw.shader.id, "max_dt"), 0.3f);
			for (const auto &data : trails.trailing_) {
				const auto to_end = (TRAIL_MAX_SEGMENTS-data.insert);
				const auto from_start = data.insert;
				glBindBuffer(GL_ARRAY_BUFFER, trails.wpos);
				glBufferSubData(GL_ARRAY_BUFFER, 0, to_end * sizeof data.buf[0], &data.buf[data.insert]);
				glBufferSubData(GL_ARRAY_BUFFER, to_end * sizeof data.buf[0], from_start * sizeof data.buf[0], &data.buf[0]          );
				glBindBuffer(GL_ARRAY_BUFFER, trails.ts);
				glBufferSubData(GL_ARRAY_BUFFER, 0, to_end * sizeof(float[2]), &data.timestamp[data.insert]);
				glBufferSubData(GL_ARRAY_BUFFER, to_end * sizeof(float[2]), from_start * sizeof(float[2]), &data.timestamp[0]          );
				glDrawElements(GL_TRIANGLES, this_draw.tricount, GL_UNSIGNED_INT, nullptr);
			}
		}
	}
}

void render::remove(entity e)
{
	const std::uint32_t idx = index(e, system_id::render);
	const std::uint32_t type_idx = idx & type_mask;
	const std::uint32_t removed_idx = idx >> type_shift;
	const std::uint32_t swapped_idx = drawing_[type_idx].size()-1;
	drawing_[type_idx][removed_idx] = drawing_[type_idx][swapped_idx];
	reindex(drawing_[type_idx][removed_idx], system_id::render, idx);
	drawing_[type_idx].pop_back();
	if (type_idx == trail) {
		trails.trailing_[removed_idx] = trails.trailing_[swapped_idx];
		trails.trailing_.pop_back();
	}
	del_component(e, system_id::render);
}

} // phobos
