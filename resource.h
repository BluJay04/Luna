/**
 * @file resource.h
 * @brief Resource Identifier Definitions for Luna.
 * @author Antigravity
 * @date 2025-11-22
 * 
 * Defines unique identifiers for application resources (icons) and 
 * menu commands.
 * 
 * Icon ID Ranges:
 * Light Awake: 100-104 (5 frames)
 * Light Sleep: 110-113 (4 frames)
 * Dark Awake:  200-204 (5 frames)
 * Dark Sleep:  210-213 (4 frames)
 */

#ifndef RESOURCE_H
#define RESOURCE_H

// Menu Command Identifiers
#define IDM_CONTEXT_EXIT    3001

// --- Light Mode Icons ---

// Awake (5 Frames)
#define IDI_LIGHT_AWAKE_0   100
#define IDI_LIGHT_AWAKE_1   101
#define IDI_LIGHT_AWAKE_2   102
#define IDI_LIGHT_AWAKE_3   103
#define IDI_LIGHT_AWAKE_4   104

// Sleep (4 Frames)
#define IDI_LIGHT_SLEEP_0   110
#define IDI_LIGHT_SLEEP_1   111
#define IDI_LIGHT_SLEEP_2   112
#define IDI_LIGHT_SLEEP_3   113

// --- Dark Mode Icons ---

// Awake (5 Frames)
#define IDI_DARK_AWAKE_0    200
#define IDI_DARK_AWAKE_1    201
#define IDI_DARK_AWAKE_2    202
#define IDI_DARK_AWAKE_3    203
#define IDI_DARK_AWAKE_4    204

// Sleep (4 Frames)
#define IDI_DARK_SLEEP_0    210
#define IDI_DARK_SLEEP_1    211
#define IDI_DARK_SLEEP_2    212
#define IDI_DARK_SLEEP_3    213

#endif // RESOURCE_H
