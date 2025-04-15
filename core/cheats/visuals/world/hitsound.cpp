
#include <Windows.h>
#include "hitsound.h"
#include <algorithm>
#include "sounds.h"
#include <imgui.h>
#include <imgui_internal.h>
#pragma comment(lib, "Winmm.lib")
using namespace hitsound;


 std::vector < hitmarker_t > m_hit_markers;
 std::vector < hitmarker_t > m_hit_markers3d ;

void c_hitmarker::on_hit(Vector_t hit_origin, int type) {
	if (type == 0) {
		hitmarker_t marker;
		marker.time = sdk::m_global_vars->m_realtime;
		marker.origin = hit_origin;
		marker.type = type;
		m_hit_markers.push_back(marker);
	}
	else {
		hitmarker_t marker;
		marker.time = sdk::m_global_vars->m_realtime;
		marker.origin = hit_origin;
		marker.type = type;
		m_hit_markers3d.push_back(marker);
	}
}

void c_hitmarker::reset() {
	if (!m_hit_markers.empty())
		m_hit_markers.clear();

	if (!m_hit_markers3d.empty())
		m_hit_markers3d.clear();
}

void c_hitmarker::run() {

	float flTime = sdk::m_global_vars->m_realtime;

	/* shot_fired stands for player_hurt event to confirm we actuall hit a player */
	if (cfg_get(bool, g_cfg.hitmarker) && !m_hit_markers.empty()) {
		for (int i = 0; i < m_hit_markers.size(); i++) {
			auto HitMarker = &m_hit_markers[i];
			if (HitMarker->type != 0)
				continue;
			// Calculate the elapsed time since the hit marker was created
			float elapsedTime = flTime - HitMarker->time;

			// If the hit marker is older than 2.0 seconds, remove it
			if (elapsedTime > 1.5f) {
				m_hit_markers.erase(m_hit_markers.begin() + i);
				continue;
			}

			const float flGap = 1.2f;
			const float flLength = 5.0f;

			ImVec2 vecScreenPosition = ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2);

			// Calculate the alpha value based on the elapsed time
			// The alpha should be 240 at 0 seconds and 0 at 2 seconds
			float alpha = 240.0f * (1.0f - (elapsedTime / 1.f)); // Linear decrease
			alpha = ImClamp(alpha, 0.0f, 240.0f); // Ensure alpha is within bounds

			ImColor color = ImColor(cfg_get(ColorPickerVar_t, g_cfg.hitmarker_col).colValue.r, cfg_get(ColorPickerVar_t, g_cfg.hitmarker_col).colValue.g, cfg_get(ColorPickerVar_t, g_cfg.hitmarker_col).colValue.b, (int)alpha);
			ImGui::GetBackgroundDrawList()->AddLine(ImVec2(vecScreenPosition.x - flGap, vecScreenPosition.y - flGap), ImVec2(vecScreenPosition.x - flLength, vecScreenPosition.y - flLength), color);
			ImGui::GetBackgroundDrawList()->AddLine(ImVec2(vecScreenPosition.x + flGap, vecScreenPosition.y + flGap), ImVec2(vecScreenPosition.x + flLength, vecScreenPosition.y + flLength), color);
			ImGui::GetBackgroundDrawList()->AddLine(ImVec2(vecScreenPosition.x + flGap, vecScreenPosition.y - flGap), ImVec2(vecScreenPosition.x + flLength, vecScreenPosition.y - flLength), color);
			ImGui::GetBackgroundDrawList()->AddLine(ImVec2(vecScreenPosition.x - flGap, vecScreenPosition.y + flGap), ImVec2(vecScreenPosition.x - flLength, vecScreenPosition.y + flLength), color);
		

		}
	}
	
	if (cfg_get(bool, g_cfg.hitmarker3d) && !m_hit_markers3d.empty()) {

		for (int i = 0; i < m_hit_markers3d.size(); i++) {
			auto HitMarker = &m_hit_markers3d[i];
			if (HitMarker->type != 1)
				continue;

			if (!HitMarker->did_dmg)
				continue;
			// Calculate the elapsed time since the hit marker was created
			float elapsedTime = flTime - HitMarker->time;

			// If the hit marker is older than 2.0 seconds, remove it
			if (elapsedTime > 4.0f) {
				HitMarker->did_dmg = false;
				m_hit_markers3d.erase(m_hit_markers3d.begin() + i);
				continue;
			}

			const float flGap = 1.0f;
			const float flLength = 5.0f;

			ImVec2 vecScreenPosition = ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2);

			// Calculate the alpha value based on the elapsed time
			// The alpha should be 240 at 0 seconds and 0 at 2 seconds
			float alpha = 240.0f * (1.0f - (elapsedTime / 3.5f)); // Linear decrease
			alpha = ImClamp(alpha, 0.0f, 240.0f); // Ensure alpha is within bounds

			if (!math::WorldToScreen(HitMarker->origin, vecScreenPosition))
				continue;

			ImColor color = ImColor(200, 200, 200, (int)alpha);
			ImGui::GetBackgroundDrawList()->AddLine(ImVec2(vecScreenPosition.x - flGap, vecScreenPosition.y - flGap), ImVec2(vecScreenPosition.x - flLength, vecScreenPosition.y - flLength), color);
			ImGui::GetBackgroundDrawList()->AddLine(ImVec2(vecScreenPosition.x + flGap, vecScreenPosition.y + flGap), ImVec2(vecScreenPosition.x + flLength, vecScreenPosition.y + flLength), color);
			ImGui::GetBackgroundDrawList()->AddLine(ImVec2(vecScreenPosition.x + flGap, vecScreenPosition.y - flGap), ImVec2(vecScreenPosition.x + flLength, vecScreenPosition.y - flLength), color);
			ImGui::GetBackgroundDrawList()->AddLine(ImVec2(vecScreenPosition.x - flGap, vecScreenPosition.y + flGap), ImVec2(vecScreenPosition.x - flLength, vecScreenPosition.y + flLength), color);
		}
	}
}
void c_sound::WavHeader_t::ParseWavHeader(unsigned char* wavData) {
	unsigned char buffer4[4];
	unsigned char buffer2[2];

	unsigned char* walker = wavData;

	// Retreive riff
	memcpy(riff, walker, sizeof(riff));
	walker += sizeof(riff);

	// overall_size
	memcpy(buffer4, walker, sizeof(buffer4));
	walker += sizeof(buffer4);

	// Convert little endian to big endian 4 byte int
	overall_size = buffer4[0] | (buffer4[1] << 8) | (buffer4[2] << 16) | (buffer4[3] << 24);

	// wave
	memcpy(wave, walker, sizeof(wave));
	walker += sizeof(wave);

	// fmt_chunk_marker
	memcpy(fmt_chunk_marker, walker, sizeof(fmt_chunk_marker));
	walker += sizeof(fmt_chunk_marker);

	// length_of_fmt
	memcpy(buffer4, walker, sizeof(buffer4));
	walker += sizeof(buffer4);

	length_of_fmt = buffer4[0] | (buffer4[1] << 8) | (buffer4[2] << 16) | (buffer4[3] << 24);

	// format_type
	memcpy(buffer2, walker, sizeof(buffer2));
	walker += sizeof(buffer2);

	format_type = buffer2[0] | (buffer2[1] << 8);

	// channels
	memcpy(buffer2, walker, sizeof(buffer2));
	walker += sizeof(buffer2);

	channels = buffer2[0] | (buffer2[1] << 8);

	// sample_rate
	memcpy(buffer4, walker, sizeof(buffer4));
	walker += sizeof(buffer4);

	sample_rate = buffer4[0] | (buffer4[1] << 8) | (buffer4[2] << 16) | (buffer4[3] << 24);

	// byterate
	memcpy(buffer4, walker, sizeof(buffer4));
	walker += sizeof(buffer4);

	byterate = buffer4[0] | (buffer4[1] << 8) | (buffer4[2] << 16) | (buffer4[3] << 24);

	// block_align
	memcpy(buffer2, walker, sizeof(buffer2));
	walker += sizeof(buffer2);

	block_align = buffer2[0] | (buffer2[1] << 8);

	// bits_per_sample
	memcpy(buffer2, walker, sizeof(buffer2));
	walker += sizeof(buffer2);

	bits_per_sample = buffer2[0] | (buffer2[1] << 8);

	// data_chunk_header
	memcpy(data_chunk_header, walker, sizeof(data_chunk_header));
	walker += sizeof(data_chunk_header);

	// data_size
	memcpy(buffer4, walker, sizeof(buffer4));
	walker += sizeof(buffer4);

	data_size = buffer4[0] | (buffer4[1] << 8) | (buffer4[2] << 16) | (buffer4[3] << 24);

	data = walker;
}

bool c_sound::AdjustHitsoundVolume(c_sound::WavHeader_t& header, float volume)
{
	// We can only adjust PMC (Uncompressed) WAV files
	if (header.format_type != 1)
		return false;

	// Adjust volume to correct ratio
	float finalVolume = std::clamp(volume, 0.f, 1.f);

	// Loop through all samples and adjust their volumes
	int numOfSamples = header.data_size / header.channels;
	unsigned char* walker = header.data;

	for (int i = 0; i < numOfSamples; i++) {
		switch (header.channels) {
			case sizeof(unsigned char) :
				// Mono
				*walker = (unsigned char)(*walker * finalVolume);
				walker += sizeof(unsigned char);
				break;
				case sizeof(short) :
					// Stereo
					*(short*)walker = (short)(*(short*)walker * finalVolume);
					walker += sizeof(short);
					break;
					case sizeof(int) :
						// Surround?
						*(int*)walker = (int)(*(int*)walker * finalVolume);
						walker += sizeof(int);
						break;
					default:
						return false;
		}
	}

	return true;
}

void c_sound::run()
{
	if (m_should_parse_hitsound) {
		parsed_hitsound = skeet;

		c_sound::WavHeader_t header;
		header.ParseWavHeader(parsed_hitsound);
		AdjustHitsoundVolume(header, cfg_get(float, g_cfg.hitmarker_expire));

		m_should_parse_hitsound = false;
	}

	if (parsed_hitsound) {
		PlaySoundA(LPCSTR(parsed_hitsound), NULL, SND_ASYNC | SND_MEMORY);
	}
}