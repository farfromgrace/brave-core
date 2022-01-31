/* copyright (c) 2022 the brave authors. all rights reserved.
 * this source code form is subject to the terms of the mozilla public
 * license, v. 2.0. if a copy of the mpl was not distributed with this file,
 * you can obtain one at http://mozilla.org/mpl/2.0/. */

#include "brave/components/brave_wallet/browser/solana_transaction.h"

#include "base/base64.h"
#include "brave/components/brave_wallet/browser/keyring_service.h"
#include "brave/components/brave_wallet/browser/solana_instruction.h"
#include "brave/components/brave_wallet/browser/solana_utils.h"

namespace brave_wallet {

SolanaTransaction::SolanaTransaction(
    const std::string& recent_blockhash,
    const std::string& fee_payer,
    const std::vector<SolanaInstruction>& instructions)
    : message_(recent_blockhash, fee_payer, instructions) {}

std::string SolanaTransaction::GetSignedTransaction(
    KeyringService* keyring_service,
    const std::string& recent_blockhash) {
  if (!recent_blockhash.empty())
    message_.SetRecentBlockHash(recent_blockhash);

  std::vector<std::string> signers;
  auto message_bytes = message_.Serialize(&signers);
  if (!message_bytes || signers.empty())
    return "";

  std::vector<uint8_t> transaction_bytes;
  // Compact array of signatures.
  CompactU16Encode(signers.size(), &transaction_bytes);
  for (const auto& signer : signers) {
    std::vector<uint8_t> signature = keyring_service->SignMessage(
        mojom::kSolanaKeyringId, signer, message_bytes.value());
    transaction_bytes.insert(transaction_bytes.end(), signature.begin(),
                             signature.end());
  }

  // Message.
  transaction_bytes.insert(transaction_bytes.end(), message_bytes->begin(),
                           message_bytes->end());

  return base::Base64Encode(transaction_bytes);
}

}  // namespace brave_wallet
