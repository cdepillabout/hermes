<h1 align="left">
<img src="https://raw.githubusercontent.com/velveteer/hermes/master/wings.svg" height=25 width=50 />
hermes
</h1>
<p align="left">
<a href="https://github.com/velveteer/hermes/actions">
  <img src="https://img.shields.io/github/actions/workflow/status/velveteer/hermes/ci.yaml?branch=master&style=for-the-badge" alt="CI badge" />
</a>
<a href="https://hackage.haskell.org/package/hermes-json">
  <img src="https://img.shields.io/hackage/v/hermes-json?label=hackage&style=for-the-badge" alt="Hackage badge" />
</a>
</p>

A Haskell interface over the [simdjson](https://github.com/simdjson/simdjson) C++ library for decoding JSON documents. Hermes, messenger of the gods, was the maternal great-grandfather of Jason, son of Aeson.

## Overview

This library exposes functions that can be used to write decoders for JSON documents using the simdjson On Demand API. From the simdjson On Demand design documentation:

> Good applications for the On Demand API might be:

> You are working from pre-existing large JSON files that have been vetted. You expect them to be well formed according to a known JSON dialect and to have a consistent layout. For example, you might be doing biomedical research or machine learning on top of static data dumps in JSON.

> Both the generation and the consumption of JSON data is within your system. Your team controls both the software that produces the JSON and the software the parses it, your team knows and control the hardware. Thus you can fully test your system.

> You are working with stable JSON APIs which have a consistent layout and JSON dialect.

With this in mind, `Data.Hermes` parsers can decode Haskell types faster than traditional `Data.Aeson.FromJSON` instances, especially in cases where you only need to decode a subset of the document. This is because `Data.Aeson.FromJSON` converts the entire document into a `Data.Aeson.Value`, which means memory usage increases linearly with the input size. The `simdjson::ondemand` API does not have this constraint because it iterates over the JSON string in memory without constructing an intermediate tree. This means decoders are truly lazy and you only pay for what you use.

For an incremental JSON parser in Haskell, see [json-stream](https://hackage.haskell.org/package/json-stream).

## Usage

This library does _not_ offer a Haskell API over the entire simdjson On Demand API. It currently binds only to what is needed for defining and running a `Decoder`. You can see the tests and benchmarks for example usage. `simdjson::ondemand` exceptions will be caught and re-thrown with enough information to troubleshoot. In the worst case you may run into a segmentation fault that is not caught, which you are encouraged to report as a bug.

### Decoders

```haskell
import qualified Data.ByteString as BS
import qualified Data.Hermes as H

personDecoder :: H.Decoder Person
personDecoder = H.withObject $ \obj ->
  Person
    <$> H.atKey "_id" H.text obj
    <*> H.atKey "index" H.int obj
    <*> H.atKey "guid" H.text obj
    <*> H.atKey "isActive" H.bool obj
    <*> H.atKey "balance" H.text obj
    <*> H.atKey "picture" (H.nullable H.text) obj
    <*> H.atKey "latitude" H.scientific obj

-- Decode a strict ByteString.
decodePersons :: BS.ByteString -> Either H.HermesException [Person]
decodePersons = H.decodeEither $ H.list personDecoder
```
### Aeson Integration

While it is not recommended to use hermes if you need the full DOM, we still provide a performant interface to decode aeson `Value`s. See an example of this in the `hermes-aeson` subpackage. Ideally, you could use hermes to selectively decode aeson `Value`s on demand, for example:

```haskell
> H.decodeEither (H.atPointer "/statuses/99/user/screen_name" H.hValueToAeson) twitter
Right (String "2no38mae")
```

### Exceptions

When decoding fails for a known reason, you will get a `Left HermesException` indicating if the error came from `simdjson` or from an internal `hermes` call.

```haskell
> decodeEither (withObject . atKey "hello" $ list text) "{ \"hello\": [\"world\", false] }"
Left (SIMDException (DocumentError {path = "/hello/1", errorMsg = "Error while getting value of type text. The JSON element does not have the requested type."))
```

## Benchmarks
We benchmark the following operations using both `hermes-json` and `aeson` strict ByteString decoders:
* Decode an array of 1 million 3-element arrays of doubles
* Full decoding of a large-ish (12 MB) JSON array of Person objects
* Partial decoding of Twitter status objects to highlight the on-demand benefits
* Decoding entire documents into `Data.Aeson.Value`

### Specs

* GHC 9.4.4
* aeson-2.1.2.1 (using `Data.Aeson.Decoding`) with text-2.0.2
* Apple M1 Pro

#### Arrays
![](https://raw.githubusercontent.com/velveteer/hermes/master/hermes-bench/arrays.svg)

<!-- AUTO-GENERATED-CONTENT:START (ARRAYS) -->
| Name                     | Mean (ps)     | 2*Stdev (ps) | Allocated  | Copied     | Peak Memory |
| ------------------------ | ------------- | ------------ | ---------- | ---------- | ----------- |
| All.Decode.Arrays.Hermes | 273653250000  | 15205262246  | 503599500  | 439094828  | 541065216   |
| All.Decode.Arrays.Aeson  | 2218769800000 | 17033918638  | 7095177999 | 2344539338 | 1241513984  |
|                          |
<!-- AUTO-GENERATED-CONTENT:END (ARRAYS) -->

#### Partial
![](https://raw.githubusercontent.com/velveteer/hermes/master/hermes-bench/partial.svg)

<!-- AUTO-GENERATED-CONTENT:START (PARTIAL) -->
| Name                                  | Mean (ps)  | 2*Stdev (ps) | Allocated | Copied  | Peak Memory |
| ------------------------------------- | ---------- | ------------ | --------- | ------- | ----------- |
| All.Decode.Partial Twitter.Hermes     | 259343457  | 24015456     | 286819    | 2463    | 94371840    |
| All.Decode.Partial Twitter.JsonStream | 2150687500 | 111820868    | 15259529  | 270063  | 110100480   |
| All.Decode.Partial Twitter.Aeson      | 4381687500 | 360705714    | 12538008  | 4632553 | 110100480   |
|                                       |
<!-- AUTO-GENERATED-CONTENT:END (PARTIAL) -->

#### Persons
![](https://raw.githubusercontent.com/velveteer/hermes/master/hermes-bench/persons.svg)

<!-- AUTO-GENERATED-CONTENT:START (PERSONS) -->
| Name                                    | Mean (ps)    | 2*Stdev (ps) | Allocated | Copied    | Peak Memory |
| --------------------------------------- | ------------ | ------------ | --------- | --------- | ----------- |
| All.Decode.Persons.Hermes               | 45604575000  | 4137396578   | 121553446 | 55640469  | 160432128   |
| All.Decode.Persons.Aeson                | 135784600000 | 9435983258   | 357269946 | 188159609 | 273678336   |
| All.Decode.Persons (Aeson Value).Hermes | 108284075000 | 5724445824   | 293956167 | 134278262 | 273678336   |
| All.Decode.Persons (Aeson Value).Aeson  | 121856250000 | 8402584442   | 286355331 | 174336922 | 273678336   |
|                                         |
<!-- AUTO-GENERATED-CONTENT:END (PERSONS) -->

## Performance Tips

* Use `text` >= 2.0 to benefit from its UTF-8 implementation.
* Decode to `Text` instead of `String` wherever possible!
* Decode to `Int` or `Double` instead of `Scientific` if you can.
* Decode your object fields in order. If encoding with `aeson`, you can leverage `toEncoding` to enforce ordering.

If you need to decode in tight loops or long-running processes (like a server), consider using the `withHermesEnv/mkHermesEnv` and `parseByteString` functions instead of `decodeEither`. This ensures the simdjson instances are not re-created on each decode. Please see the [simdjson performance docs](https://github.com/simdjson/simdjson/blob/master/doc/performance.md#performance-notes) for more info. But please ensure that you use one `HermesEnv` per thread, as simdjson is [single-threaded by default](https://github.com/simdjson/simdjson/blob/master/doc/basics.md#thread-safety).

## Limitations

Because the On Demand API uses a forward-only iterator (except for object fields), you must be mindful to not access values out of order. This library tries to prevent this as much as possible, i.e. making `Decoder Value` impossible.

Because the On Demand API does not validate the entire document upon creating the iterator (besides UTF-8 validation and basic well-formed checks), it is possible to parse an invalid JSON document but not realize it until later. If you need the entire document to be validated up front then a DOM parser is a better fit for you.

> The On Demand approach is less safe than DOM: we only validate the components of the JSON document that are used and it is possible to begin ingesting an invalid document only to find out later that the document is invalid. Are you fine ingesting a large JSON document that starts with well formed JSON but ends with invalid JSON content?

This library currently cannot decode scalar documents, e.g. a single string, number, boolean, or null as a JSON document.

## Portability

Per the `simdjson` documentation:

> A recent compiler (LLVM clang6 or better, GNU GCC 7.4 or better, Xcode 11 or better) on a 64-bit (PPC, ARM or x64 Intel/AMD) POSIX systems such as macOS, freeBSD or Linux. We require that the compiler supports the C++11 standard or better.

However, this library relies on `std::string_view` without a shim, so C++17 or better is highly recommended.
