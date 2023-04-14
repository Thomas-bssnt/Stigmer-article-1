from __future__ import annotations

from pathlib import Path

from modules.files import get_files_sessions
from modules.game import Game


class ValuesDict(dict):
    """Dictionary that is iterated over values."""

    def __iter__(self):
        return (v for v in self.values())

    def __repr__(self) -> str:
        return str(list(self.values()))


class Games(ValuesDict):
    """"""

    def __init__(self, path_data: Path | None = None, game_type: str | None = None, /) -> None:
        if path_data is not None and game_type is not None:
            self.session: dict[str:Games] = ValuesDict()
            for files_session in get_files_sessions(path_data, game_type):
                games_session = self.from_files(files_session)

                game = list(games_session.values())[0]
                players = {player["playerPseudo"] for player in game.players}
                self.session[(game.session, frozenset(players))] = games_session
                # self.session[list(games_session.values())[0].session] = games_session

                self |= games_session

    def __repr__(self) -> str:
        return f"Games({super().__repr__()})"

    @classmethod
    def from_files(cls, files: list[tuple[Path, Path]]) -> Games:
        games = Games()
        for in_file, out_file in files:
            game = Game(in_file, out_file)
            games[game.id] = game
        return games


if __name__ == "__main__":

    from constants import PATH_DATA

    games = Games(PATH_DATA / "rule_1", "Group_R1")

    print(games)
    print()

    for game in games:
        print(game, game.session)
        break
    print()

    for games_session in games.session:
        for game in games_session:
            print(game)
            break
        break
    print()

    for session, games_session in games.session.items():
        for game in games_session:
            print(session, game)
            break
        break
    print()

    print(games.keys())
    print(games.session["01_a"].keys())
