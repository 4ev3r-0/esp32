local lines = load_text_file("/splash_anim/logo.txt")
if lines then
    local currentY = 40
    for i = 1, #lines do
        lcd_print(lines[i], 10, currentY)
        currentY = currentY + 20
    end
end

function update()
end

