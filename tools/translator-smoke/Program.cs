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
const uint addiR3R3Seven = 0x38630007;
const uint blr = 0x4E800020;
var memory = new byte[0x1000];
BinaryPrimitives.WriteUInt32BigEndian(memory.AsSpan(0, 4), addiR3R3Seven);
BinaryPrimitives.WriteUInt32BigEndian(memory.AsSpan(4, 4), blr);

var usedRange = new AddressRange(entryPoint, entryPoint + 8);
var image = new ProgramImage(
    memory,
    usedRange,
    usedRange,
    new AddressRange(0, 0),
    Convert.ToHexString(SHA256.HashData(memory)),
    memoryBase: entryPoint);

var translator = new FunctionTranslator(image);
var result = translator.Translate(entryPoint, new TranslationOptions(
    PreferredName: "wii_compiled_translated_addi_smoke",
    EnableLeafInlining: false));

if (result.Instructions.Count != 2 || result.Metrics.UnsupportedInstructionCount != 0)
{
    Console.Error.WriteLine(
        $"Unexpected translation: instructions={result.Instructions.Count}, unsupported={result.Metrics.UnsupportedInstructionCount}");
    return 3;
}

const string upstreamAbiInclude = "#include \"guest_abi/guest_abi.h\"";
const string browserAbiInclude = "#include \"browser_guest_abi_smoke.h\"";
if (!result.CxxCode.Contains(upstreamAbiInclude, StringComparison.Ordinal))
{
    Console.Error.WriteLine("Translator output did not contain the expected upstream guest ABI include.");
    return 4;
}

var browserCpp = result.CxxCode.Replace(upstreamAbiInclude, browserAbiInclude, StringComparison.Ordinal);
if (!browserCpp.Contains("wii_compiled_translated_addi_smoke", StringComparison.Ordinal))
{
    Console.Error.WriteLine("Translator output did not contain the requested function symbol.");
    return 5;
}

var output = Path.GetFullPath(args[0]);
Directory.CreateDirectory(Path.GetDirectoryName(output)!);
File.WriteAllText(output, browserCpp);
Console.WriteLine("Upstream WiiCompiled translator generated C++ from synthetic PowerPC instructions.");
Console.WriteLine($"  entry: 0x{entryPoint:X8}");
Console.WriteLine($"  PPC instructions: {result.Metrics.PpcInstructionCount}");
Console.WriteLine($"  unsupported instructions: {result.Metrics.UnsupportedInstructionCount}");
Console.WriteLine("  ABI include: replaced desktop guest_abi umbrella with matching minimal CPU-context shim");
Console.WriteLine($"  generated C++ bytes: {browserCpp.Length}");
Console.WriteLine($"  output: {output}");
return 0;
