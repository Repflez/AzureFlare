#pragma once

// Typedef for the GameRegion function
typedef bool (*OriginalIsEpisode4)();

void PatchEpisode4Mode(int);
void UnpatchEpisode4Mode();