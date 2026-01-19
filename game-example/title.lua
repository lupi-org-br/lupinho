function make_title()
    local move_to_next = 0
    return {
        name = function() return "title" end,
        update = function() end,
        is_finished = function()
            return move_to_next == 18
        end,
        draw = function(current_frame)
            frame = current_frame
            ui.set_pallet(1, 256, Pallets.aurora)

            pad_1, pad_2 = sys.get_controller_state()

            if pad_1 and pad_1.l == kState.pressed and pad_1.r == kState.pressed then
                sys.change_game("shell")
            end

            if move_to_next > 0 then
                move_to_next = move_to_next + 1
            end

            if pad_1.start == kState.press or pad_1.a == kState.press then
                move_to_next = 1
            end

            ui.preload_spritesheet(SpriteSheets['poi_cherry_' .. 1 + (frame // 4) % 7])
            ui.draw_rect(0, 0, 480, 270, true, 31, false, 0)

            local delta = (current_frame // 2) % 32
            for x = 0, 21, 1 do
                for y = 0, 12, 1 do
                    local next_x = x * 16
                    local next_y = y * 16 + 8 - delta
                    local next_radius = ((x + y) % 2 == 0) and 8 or 4
                    if next_y >= -16 and next_y < 270 + 16 then
                        ui.draw_circle(next_x, next_y, next_radius, true, 32, false, 0)
                    end
                end
            end

            ui.draw_rect(22, 22, 302, 162, true, 8, false, 0)
            ui.draw_rect(20, 20, 300, 160, true, 7, false, 0)

            ui.draw_text("\1\9Encontre as cerejas !", 112, 50)
            ui.draw_text("\1\15 15 x", 138, 80)
            ui.draw_sprite(160, 70, 0, kSpriteSize.s32x32)

            local delta = nil
            if move_to_next > 0 then
                delta = math.min(move_to_next / 2, 8) // 1
            elseif frame < 32 then
                delta = math.min((32 - frame) / 2, 8) // 1
            end

            if delta ~= nil then
                for x = 0, 21, 1 do
                    for y = 0, 12, 1 do
                        local next_x = x * 16
                        local next_y = y * 16
                        ui.draw_rect(
                            next_x + 8 - delta,
                            next_y + 8 - delta,
                            next_x + 8 + delta,
                            next_y + 8 + delta,
                            true, 8, false, 0)
                    end
                end
            end
        end
    }
end
