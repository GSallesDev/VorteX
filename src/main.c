/* VorteX - Racing Game
 * Copyright (C) 2026 Gustavo Henrique de Sales
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "app.h"


int main(void) {
    AppState app = { 0 };

    if (!app_init(&app)) return 1;

    while (!app_should_close(&app)) {
        app_frame(&app);
    }

    app_shutdown(&app);
    return 0;
}
