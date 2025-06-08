# Copyright (C) 2025 Konstantinos Chousos
#
# This file is part of LLM-Harness.
#
# LLM-Harness is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# LLM-Harness is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with LLM-Harness.  If not, see <https://www.gnu.org/licenses/>.

"""
Data models for project analysis.
"""

from dataclasses import dataclass


@dataclass
class ProjectFile:
    """Represents a source file in the project."""

    path: str
    name: str
    content: str


@dataclass
class ProjectInfo:
    """Contains information about a project."""

    source: list[ProjectFile]
    static: str | None = None

    def get_concatenated_content(self) -> str:
        """
        Returns the concatenated content of all project files.

        Returns:
            str: Contents of all project files.
        """
        file_contents = []

        for file in self.source:
            file_contents.append(f"\n>>>> {file.name}\n")
            file_contents.append(file.content)

        return "".join(file_contents)

    def get_static_analysis(self) -> str | None:
        return self.static
