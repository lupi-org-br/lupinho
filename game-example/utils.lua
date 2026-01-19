function DrawTrisFade(current_frame, progress, stage)
    local percent = progress
    local progress = (96 * progress) // 1
    local size = 45 + progress * 8

    for i = -1, size // 4 do
        local f = progress > 0 and 0 or (current_frame // 4)
        local dx = 5 * i - f % 5
        local dy = size - 5 * i + f % 5
        if dx < 480 and dy < 270 then
            ui.trisfill(dx, 5 + dy, 5 + dx, dy, 5 + dx, 5 + dy, kColors.purple_dark)
        end
    end

    ui.trisfill(0, -1, 0, size + 6, size + 6, -1, kColors.purple_dark)
    local ex = lerp(1, 220, math.min(1.0, 2 * percent)) // 1
    local ey = (ex * 0.5) // 1

    ui.print(CurrentStage.display_name, ex, ey + 1, kColors.black)
    ui.print(CurrentStage.display_name, ex + 1, ey, kColors.green_light)
    ui.print("(" .. CurrentStage.cherries .. ")", (ex * 1.0) // 1, ey + 12, kColors.gray_light)
end
