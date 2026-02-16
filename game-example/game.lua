-- Setup palette colors
-- Format: palset(index, BGR555 value)
-- BGR555: 5 bits each for Blue, Green, Red (15-bit color)
ui.palset(0, 0x0000) -- Black/Transparent
ui.palset(1, 0x7FFF) -- White
ui.palset(2, 0x001F) -- Red
ui.palset(3, 0x03E0) -- Green
ui.palset(4, 0x7C00) -- Blue
ui.palset(5, 0x7FE0) -- Cyan
ui.palset(6, 0x03FF) -- Yellow
ui.palset(7, 0x7C1F) -- Magenta

-- Animation variables
local time = 0
local screenWidth = 480
local screenHeight = 270

function update()
	-- Clear screen with black

	time = time + 1

	-- Draw animated diagonal lines
	local offset = (time * 2) % 80
	for i = 0, 10 do
		local x = i * 40 + offset
		ui.draw_line(x, 0, x + screenHeight, screenHeight, i % 7 + 1)
	end
end
