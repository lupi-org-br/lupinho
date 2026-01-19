require "sprites"

function lerp(start, ending, amount)
    return (1 - amount) * start + amount * ending
end

function ping_pong(current_frame, max_value)
    local cycle_length = 2 * (max_value - 1)
    local position_in_cycle = current_frame % cycle_length

    if position_in_cycle < max_value then
        return position_in_cycle + 1
    else
        return 2 * max_value - position_in_cycle - 1
    end
end

kGravity = 0.3
kMaxGravity = 4
kJumpForce = 5.2
kPlayerSpeed = 2

kSpriteSize = {
    s8x8 = 1,
    s16x16 = 2,
    s32x32 = 3,
    s64x64 = 4,
    s64x160 = 5
}

kState = {
    idle = 0,
    press = 1,
    pressed = 2
}

kPlayerStates = {
    climb = { asset = 'climb', frames = 3, cadency = 12 },
    crouch = { asset = 'crouch', frames = 2, cadency = 12 },
    hurt = { asset = 'hurt', frames = 2, cadency = 12 },
    idle = { asset = 'idle', frames = 4, cadency = 8 },
    jump = { asset = 'jump', frames = 1, cadency = 12 },
    fall = { asset = 'fall', frames = 1, cadency = 12 },
    run = { asset = 'run', frames = 6, cadency = 4 },
    dead = { asset = 'hurt', frames = 2, cadency = 6 },
}

kMapID = {
    tile = 1,
    colision = 2,
    poi = 3,
    overlay = 4,
}

kColisionType = {
    top = 1,
    bottom = 2,
    left = 3,
    right = 4
}

kColisionTile = {
    [122] = { kColisionType.top, kColisionType.bottom, kColisionType.left, kColisionType.right },
    [123] = { kColisionType.top, kColisionType.left, kColisionType.right },
    [124] = { kColisionType.bottom }
}

kPoiType = {
    cherry = 115,
    spike = 114,
    spring = 116,
    beetle = 117,
    palm = 141,
    pine = 137,
    bird = 119,
    house = 155
}

kColors = {
    black = 6,
    purple_light = 22,
    purple_dark = 31,
    green_dark = 76,
    green_light = 75,
    green_bg = 69,
    gray_light = 27,
    red_light = 16,
    blue_light = 79,
    yellow = 26
}

kStages = {

    {
        map_location = { x = 31, y = 36 },
        player_start = { x = 7, y = 14 },
        looking_back = false,
        bg_up        = SpriteSheets["tilemap.leaves"],
        bg_down      = SpriteSheets["tilemap.forest"],
        cherries     = 2,
        bg_tint      = kColors.green_bg,
        display_name = "X 1-1",
        map_name     = "new_w1s1"
    },{
        map_location = { x = 36, y = 35 },
        player_start = { x = 4, y = 2 },
        looking_back = false,
        bg_up        = SpriteSheets["tilemap.deep"],
        bg_down      = SpriteSheets["tilemap.rocks"],
        cherries     = 2,
        bg_tint      = 65,
        display_name = "X 1-2",
        map_name     = "new_w1s2"
    },{
        map_location = { x = 40, y = 33 },
        player_start = { x = 6, y = 0 },
        looking_back = false,
        bg_up        = SpriteSheets["tilemap.deep"],
        bg_down      = SpriteSheets["tilemap.rocks"],
        cherries     = 2,
        bg_tint      = 65,
        display_name = "X 1-3",
        map_name     = "new_w1s3"
    },{
        map_location = { x = 33, y = 30 },
        player_start = { x = 3, y = 4 },
        looking_back = false,
        bg_up        = SpriteSheets["tilemap.leaves"],
        bg_down      = SpriteSheets["tilemap.forest"],
        cherries     = 1,
        bg_tint      = kColors.green_bg,
        display_name = "X 1-4",
        map_name     = "new_w1s4"
    },{
        map_location = { x = 48, y = 31 },
        player_start = { x = 3, y = 6 },
        looking_back = false,
        bg_up        = SpriteSheets["tilemap.deep"],
        bg_down      = SpriteSheets["tilemap.rocks"],
        cherries     = 2,
        bg_tint      = 65,
        display_name = "X 1-5",
        map_name     = "new_w1s5"
    },{
        map_location = { x = 52, y = 26 },
        player_start = { x = 3, y = 6 },
        looking_back = false,
        bg_up        = SpriteSheets["tilemap.deep"],
        bg_down      = SpriteSheets["tilemap.rocks"],
        cherries     = 1,
        bg_tint      = 65,
        display_name = "X 1-6",
        map_name     = "new_w1s6"
    },{
        map_location = { x = 51, y = 22 },
        player_start = { x = 2, y = 5 },
        looking_back = false,
        bg_up        = SpriteSheets["tilemap.deep"],
        bg_down      = SpriteSheets["tilemap.rocks"],
        cherries     = 1,
        bg_tint      = 65,
        display_name = "X 1-7",
        map_name     = "new_w1s7"
    },{
        map_location = { x = 55, y = 24 },
        player_start = { x = 4, y = 45 },
        looking_back = false,
        bg_up        = SpriteSheets["tilemap.clouds"],
        bg_down      = SpriteSheets["tilemap.ocean"],
        cherries     = 5,
        bg_tint      = kColors.blue_light,
        display_name = "X 1-8",
        map_name     = "new_w1s8"
    },{
        map_location = { x = 44, y = 23 },
        player_start = { x = 5, y = 14 },
        looking_back = false,
        bg_up        = SpriteSheets["tilemap.clouds"],
        bg_down      = SpriteSheets["tilemap.ocean"],
        cherries     = 5,
        bg_tint      = kColors.blue_light,
        display_name = "X 1-9",
        map_name     = "new_w1s9"
    }
}

CurrentStage = kStages[2]
