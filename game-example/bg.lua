function make_bg()
    return {
        before_frame = function(frame, camera, player, map)

        end,
        on_frame = function(frame, camera, player, map)
            local camx, camy = camera.getxy()
            local dx = (camx // 5)
            local dy = (math.min(1, (camy / (30 * 16))) * 16) // 1
            local tile_id = 0

            ui.rectfill(0, 0, 480, 270, CurrentStage.bg_tint)
            local tileset = CurrentStage.bg_down

            for x = 0, 32 do
                tile_id = ((x * 16 + dx) // 16) % 10
                ui.tile(tileset, tile_id, x * 16 - (dx % 16), 165 - dy)
            end

            tileset = CurrentStage.bg_up
            dx = (camx // 4)

            for x = 0, 32 do
                tile_id = ((x * 16 + dx) // 16) % 15
                ui.tile(tileset, tile_id, x * 16 - (dx % 16), 0)
            end
        end
    }
end
