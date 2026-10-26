using System.Text.Json;

namespace Citadels.Client;

/// <summary>Turns one line of the engine's output into an <see cref="EngineMessage"/>.</summary>
public static class ProtocolParser
{
    /// <exception cref="ProtocolException">The line is not valid JSON or a field is missing or has the wrong type.</exception>
    public static EngineMessage Parse(string line)
    {
        ArgumentNullException.ThrowIfNull(line);

        JsonDocument document;
        try
        {
            document = JsonDocument.Parse(line);
        }
        catch (JsonException ex)
        {
            throw new ProtocolException("The engine sent a line that is not valid JSON: " + Shorten(line), ex);
        }

        using (document)
        {
            JsonElement root = document.RootElement;
            if (root.ValueKind != JsonValueKind.Object)
            {
                throw new ProtocolException("The engine sent JSON that is not an object: " + Shorten(line));
            }

            string type = GetString(root, "type");
            switch (type)
            {
                case "announce":
                    return new AnnounceMessage(GetString(root, "text"));
                case "handOff":
                    return new HandOffMessage(GetString(root, "name"), GetInt(root, "seat"));
                case "choose":
                    return new ChooseMessage(GetString(root, "prompt"), GetStrings(root, "options"));
                case "confirm":
                    return new ConfirmMessage(GetString(root, "prompt"));
                case "table":
                    return new TableMessage(GetInt(root, "round"), GetInt(root, "crown"), GetPlayers(root, "players"));
                case "private":
                    return new PrivateMessage(
                        GetString(root, "name"), GetInt(root, "seat"), GetInt(root, "gold"), GetCards(root, "hand"));
                case "error":
                    return new ErrorMessage(GetString(root, "text"));
                default:
                    return new UnknownMessage(type, line);
            }
        }
    }

    private static JsonElement Require(JsonElement parent, string name, JsonValueKind kind)
    {
        if (parent.ValueKind != JsonValueKind.Object
            || !parent.TryGetProperty(name, out JsonElement value)
            || value.ValueKind != kind)
        {
            throw new ProtocolException($"Field '{name}' is missing or has the wrong type.");
        }

        return value;
    }

    private static string GetString(JsonElement parent, string name)
    {
        return Require(parent, name, JsonValueKind.String).GetString() ?? string.Empty;
    }

    private static int GetInt(JsonElement parent, string name)
    {
        JsonElement value = Require(parent, name, JsonValueKind.Number);
        if (!value.TryGetInt32(out int number))
        {
            throw new ProtocolException($"Field '{name}' is not a whole number.");
        }

        return number;
    }

    private static List<string> GetStrings(JsonElement parent, string name)
    {
        var result = new List<string>();
        foreach (JsonElement item in Require(parent, name, JsonValueKind.Array).EnumerateArray())
        {
            if (item.ValueKind != JsonValueKind.String)
            {
                throw new ProtocolException($"Field '{name}' must contain only strings.");
            }

            result.Add(item.GetString() ?? string.Empty);
        }

        return result;
    }

    private static List<Card> GetCards(JsonElement parent, string name)
    {
        var result = new List<Card>();
        foreach (JsonElement item in Require(parent, name, JsonValueKind.Array).EnumerateArray())
        {
            result.Add(ParseCard(item));
        }

        return result;
    }

    private static Card ParseCard(JsonElement element)
    {
        string? ability = null;
        if (element.ValueKind == JsonValueKind.Object && element.TryGetProperty("ability", out JsonElement abilityValue))
        {
            if (abilityValue.ValueKind == JsonValueKind.String)
            {
                ability = abilityValue.GetString();
            }
            else if (abilityValue.ValueKind != JsonValueKind.Null)
            {
                throw new ProtocolException("Field 'ability' must be a string or null.");
            }
        }

        return new Card(
            GetString(element, "name"),
            GetInt(element, "cost"),
            ParseColor(GetString(element, "color")),
            ability);
    }

    private static List<PlayerSummary> GetPlayers(JsonElement parent, string name)
    {
        var result = new List<PlayerSummary>();
        foreach (JsonElement item in Require(parent, name, JsonValueKind.Array).EnumerateArray())
        {
            result.Add(new PlayerSummary(
                GetInt(item, "seat"),
                GetString(item, "name"),
                GetInt(item, "gold"),
                GetInt(item, "handSize"),
                GetCards(item, "city")));
        }

        return result;
    }

    private static CardColor ParseColor(string word)
    {
        switch (word)
        {
            case "noble": return CardColor.Noble;
            case "trade": return CardColor.Trade;
            case "religious": return CardColor.Religious;
            case "military": return CardColor.Military;
            case "special": return CardColor.Special;
            default: throw new ProtocolException($"Unknown colour '{word}'.");
        }
    }

    private static string Shorten(string line)
    {
        const int limit = 120;
        return line.Length <= limit ? line : line.Substring(0, limit) + "...";
    }
}
