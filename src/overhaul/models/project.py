# Copyright (C) 2025 Konstantinos Chousos
#
# This file is part of OverHAuL.
#
# OverHAuL is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# OverHAuL is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with OverHAuL.  If not, see <https://www.gnu.org/licenses/>.

"""
Data models for project analysis.
"""

from dataclasses import dataclass


@dataclass
class ProjectInfo:
    """Contains information about a project."""

    static: str | None = None
    error: str | None = None
    harness: str | None = None
    output: str | None = None
    compiles: bool = False

    def get_static_analysis(self) -> str | None:
        return self.static

    def get_error(self) -> str | None:
        return self.error

    def get_harness(self) -> str | None:
        return self.harness

    def get_output(self) -> str | None:
        return self.output

    def get_compilation_status(self) -> bool:
        return self.compiles
