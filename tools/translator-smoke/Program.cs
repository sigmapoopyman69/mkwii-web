using System.Buffers.Binary;
using System.Security.Cryptography;
using Translator.Core.Loading;
using Translator.Core.Translation;

if (args.Length != 1)
{
    Console.Error.WriteLine("Usage: TranslatorSmoke <generated-cpp-output>");
    return 2;
}

const uint entryPoint = 0x80001000;
const uint branchEntry = entryPoint;
const uint memoryEntry = entryPoint + 0x30;

// branch function:
//   cmpwi r3, 42
//   beq   +12
//   addi  r4, 0, 9
//   blr
//   addi  r4, 0, 7
//   blr
//
// memory function:
//   lwz   r4, 0(r3)
//   addi  r4, r4, 1
//   stw   r4, 4(r3)
//   blr
var words = new uint[]
{
    0x2C03002A, 0x4182000C, 0x38800009, 0x4E800020, 0x38800007, 0x4E800020,
    0, 0, 0, 0, 0, 0, 0,
    0x80830000, 0x38840001, 0x90830004, 0x4E800020
};
var memory = new byte[0x1000];
for (var i = 0; i < words.Length; i++)
    BinaryPrimitives.WriteUInt32BigEndian(memory.AsSpan(i * 4, 4), words[i]);

var usedRange = new AddressRange(entryPoint, entryPoint + 0x100);
var image = new ProgramImage(
    memory,
    usedRange,
    usedRange,
    new AddressRange(0, 0),
    Convert.ToHexString(SHA256.HashData(memory)),
    memoryBase: entryPoint);

var translator = new FunctionTranslator(image);
var branchResult = translator.Translate(branchEntry, new TranslationOptions(
    PreferredName: "wii_compiled_translated_branch_smoke",
    EnableLeafInlining: false));
var memoryResult = translator.Translate(memoryEntry, new TranslationOptions(
    PreferredName: "wii_compiled_translated_memory_smoke",
    EnableLeafInlining: false));

Validate(branchResult, "branch", expectedInstructions: 6);
Validate(memoryResult, "memory", expectedInstructions: 4);

const string upstreamAbiInclude = "#include \"guest_abi/guest_abi.h\"";
const string browserAbiInclude = "#include \"browser_guest_abi_smoke.h\"";

string Adapt(string code, string symbol)
{
    if (!code.Contains(upstreamAbiInclude, StringComparison.Ordinal))
        throw new InvalidOperationException($"{symbol}: missing expected upstream guest ABI include.");
    var adapted = code.Replace(upstreamAbiInclude, browserAbiInclude, StringComparison.Ordinal);
    if (!adapted.Contains(symbol, StringComparison.Ordinal))
        throw new InvalidOperationException($"{symbol}: generated function symbol missing.");
    return adapted;
}

var generated = Adapt(branchResult.CxxCode, "wii_compiled_translated_branch_smoke")
    + Environment.NewLine
    + Adapt(memoryResult.CxxCode, "wii_compiled_translated_memory_smoke");

var output = Path.GetFullPath(args[0]);
Directory.CreateDirectory(Path.GetDirectoryName(output)!);
File.WriteAllText(output, generated);
Console.WriteLine("Upstream WiiCompiled translator generated two synthetic PowerPC functions.");
Console.WriteLine($"  branch entry: 0x{branchEntry:X8}; instructions: {branchResult.Metrics.PpcInstructionCount}; unsupported: {branchResult.Metrics.UnsupportedInstructionCount}");
Console.WriteLine($"  memory entry: 0x{memoryEntry:X8}; instructions: {memoryResult.Metrics.PpcInstructionCount}; unsupported: {memoryResult.Metrics.UnsupportedInstructionCount}");
Console.WriteLine("  coverage: cmpwi/beq conditional paths, addi, lwz, stw, blr");
Console.WriteLine($"  generated C++ bytes: {generated.Length}");
Console.WriteLine($"  output: {output}");
return 0;

static void Validate(FunctionTranslationResult result, string label, int expectedInstructions)
{
    if (result.Metrics.PpcInstructionCount != expectedInstructions || result.Metrics.UnsupportedInstructionCount != 0)
    {
        throw new InvalidOperationException(
            $"{label}: unexpected translation; PPC instructions={result.Metrics.PpcInstructionCount}, IR instructions={result.Instructions.Count}, unsupported={result.Metrics.UnsupportedInstructionCount}");
    }
}
