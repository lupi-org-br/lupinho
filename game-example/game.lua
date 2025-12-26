require "pallet"

x = 200
t = 0

for i = 1, #Palette do
    ui.palset(i - 1, Palette[i])
end

function update()
    x = x + 4
    t = t + 0.05

    y =  math.sin(t) * 25

    ui.draw_text("Bem-vindo ao Lupi!", 280, 180 + math.floor(y))

    ui.draw_line(10, 10, 280, 180, 3)
    ui.draw_rect(50, 50, 80, 80, true, 40)
    ui.draw_rect(185, 185, 50, 50, false, 5)
    ui.draw_circle(200, 300, 20, true, 6, true, 10)
    ui.draw_circle(260, 300, 20, true, 7, false, 11)
    ui.draw_circle(200, 360, 20, false, 8, true, 12)
    ui.draw_triangle(20, 250, 100, 250, 55, 350, 9)
end
