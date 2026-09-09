#include "MapCollision.h"

MapCollision::MapCollision()
{
}

MapCollision::~MapCollision()
{
}

bool MapCollision::TryMove(const std::vector<std::vector<int>>& currentMap,
                           const std::vector<std::unique_ptr<BaseObject2d>>& objects,
                           BaseObject2d& subject,
                           const Vector2Int& dir)
{
	if (dir.x == 0 && dir.y == 0)
	{
		return false;
	}

	Vector2Int nextPos = subject.GetPosition() + dir;
	int nextType = CheckType(currentMap, nextPos);

	// 1. 床（通行可能領域）への移動
	if (nextType == 0)
	{
		subject.Move(dir);
		return true;
	}

	// 2. 壁・範囲外(1) または 静的オブジェクト(3) への移動（進行不可）
	if (nextType == 1 || nextType == 3)
	{
		return false;
	}

	// 3. 動的オブジェクト(2) への移動（押し出し処理の試行）
	if (nextType == 2)
	{
		BaseObject2d* pushedObj = FindObjectAtPos(objects, nextPos);
		if (pushedObj && pushedObj->IsDynamic())
		{
			Vector2Int nextNextPos = nextPos + dir;
			int nextNextType = CheckType(currentMap, nextNextPos);

			// 押し出し先のマスが床(0)であれば、押して進むことができる
			if (nextNextType == 0)
			{
				pushedObj->Move(dir);
				subject.Move(dir);
				return true;
			}
			else if (nextNextType == 2)
			{
				// 押し出し先がさらに動的オブジェクトの場合、再帰的に押し出しを試みる
				if (TryMove(currentMap, objects, *pushedObj, dir))
				{
					subject.Move(dir);
					return true;
				}
			}
		}
		// 押し出し先が詰まっているため移動不可
		return false;
	}

	return false;
}

int MapCollision::CheckType(const std::vector<std::vector<int>>& map, const Vector2Int& pos)
{
	// 0 は通行可能、1 は壁又は範囲外、 2 は動的オブジェクト、3 は静的オブジェクト
	if (pos.y >= 0 && pos.x >= 0 &&
	    static_cast<size_t>(pos.y) < map.size() &&
	    static_cast<size_t>(pos.x) < map[pos.y].size())
	{
		return map[pos.y][pos.x] / 100;
	}
	return 1; // 範囲外は壁扱い
}

BaseObject2d* MapCollision::FindObjectAtPos(const std::vector<std::unique_ptr<BaseObject2d>>& objects, const Vector2Int& pos)
{
	for (const auto& obj : objects)
	{
		if (obj && obj->GetPosition() == pos)
		{
			return obj.get();
		}
	}
	return nullptr;
}

